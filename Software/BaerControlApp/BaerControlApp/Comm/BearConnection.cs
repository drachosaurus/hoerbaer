using HoerBaer.Ble;
using Plugin.BLE;
using Plugin.BLE.Abstractions.Contracts;

namespace BaerControlApp.Comm;

public class BearConnection : IDisposable
{
    private readonly DiscoveredDevice _device;
    private readonly IAdapter _adapter;

    public BaerState State { get; }
    
    public BearConnection(DiscoveredDevice device)
    {
        _device = device;
        _adapter = CrossBluetoothLE.Current.Adapter;
        State = new BaerState();
    }

    public async Task ConnectAndSubscribe()
    {
        var bleDevice = _device.DeviceRef;
        await _adapter.ConnectToDeviceAsync(bleDevice);
        
        var service = await bleDevice.GetServiceAsync(KnownIds.ServiceId);
        if (service == null)
            throw new InvalidOperationException("Service not found");
        
        var powerChar = await service.GetCharacteristicAsync(KnownIds.PowerCharacteristicId);
        if (powerChar == null)
            throw new InvalidOperationException("Power characteristic not found");
        
        powerChar.ValueUpdated += (o, args) => DeserializeUpdatePowerValues(args.Characteristic.Value);

        var playerChar = await service.GetCharacteristicAsync(KnownIds.PlayerCharacteristicId);
        if (playerChar == null)
            throw new InvalidOperationException("Player characteristic not found");

        playerChar.ValueUpdated += (o, args) => DeserializeUpdatePlayerValues(args.Characteristic.Value);

        await powerChar.StartUpdatesAsync();
        await playerChar.StartUpdatesAsync();
    }

    public void Dispose()
    {
        _adapter.DisconnectDeviceAsync(_device.DeviceRef);
    }

    private void DeserializeUpdatePowerValues(byte[]? bytes)
    {
        if(bytes == null || bytes.Length == 0)
            return;

        var powerStatePayload = PowerStateCharacteristic.Parser.ParseFrom(bytes);
        State.Power.BatteryPresent = powerStatePayload.BatteryPresent;
        State.Power.BatteryVoltage = powerStatePayload.BatteryVoltage;
        State.Power.BatteryPercentage = powerStatePayload.BatteryPercentage;
        State.Power.Charging = powerStatePayload.Charging;
    }

    private void DeserializeUpdatePlayerValues(byte[]? bytes)
    {
        if(bytes == null || bytes.Length == 0)
            return;

        var playerStatePayload = PlayerStateCharacteristic.Parser.ParseFrom(bytes);
        State.PlayingInfo.State = playerStatePayload.State;
        State.PlayingInfo.SlotActive = playerStatePayload.SlotActive;
        State.PlayingInfo.FileIndex = playerStatePayload.FileIndex;
        State.PlayingInfo.FileCount = playerStatePayload.FileCount;
        State.PlayingInfo.CurrentTime = playerStatePayload.CurrentTime;
        State.PlayingInfo.Duration = playerStatePayload.Duration;
        State.PlayingInfo.Volume = playerStatePayload.Volume;
        State.PlayingInfo.MaxVolume = playerStatePayload.MaxVolume;
    }
}