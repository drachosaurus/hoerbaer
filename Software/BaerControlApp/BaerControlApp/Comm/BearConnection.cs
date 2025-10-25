using System.Net;
using System.Net.Http.Json;
using System.Xml;
using HoerBaer.Ble;
using Plugin.BLE;
using Plugin.BLE.Abstractions.Contracts;
using Plugin.BLE.Abstractions.EventArgs;

namespace BaerControlApp.Comm;

public class BearConnection : IDisposable
{
    private readonly DiscoveredDevice _device;
    private readonly IAdapter _adapter;
    
    public BearState State { get; }
    
    private readonly List<(ICharacteristic characteristic, EventHandler<CharacteristicUpdatedEventArgs> eventSubscription)> _subscribedCharacteristics = new();

    public bool NetworkConnected => Slots != null;
    public List<SlotInfo>? Slots { get; private set; }

    public bool IsConnected => _device.IsConnected;
    public Guid Id => _device.Id;
    
    public event EventHandler? Disconnected;

    internal BearConnection(DiscoveredDevice device)
    {
        _device = device;
        _adapter = CrossBluetoothLE.Current.Adapter;
        State = new BearState();
        
        _adapter.DeviceDisconnected += AdapterOnDeviceDisconnected;
    }

    public async Task ConnectAndSubscribe()
    {
        var bleDevice = _device.DeviceRef;
        await _adapter.ConnectToDeviceAsync(bleDevice);
        
        var service = await bleDevice.GetServiceAsync(KnownIds.ServiceId);
        if (service == null)
            throw new InvalidOperationException("Service not found");
        
        await ReadAndSubscribeCharacteristic(service, KnownIds.PowerCharacteristicId, DeserializeUpdatePowerValues);
        await ReadAndSubscribeCharacteristic(service, KnownIds.PlayerCharacteristicId, DeserializeUpdatePlayerValues);
        await ReadAndSubscribeCharacteristic(service, KnownIds.NetworkCharacteristicId, DeserializeUpdateNetworkValues);

        await DownloadTracksIfConnected();
    }

    private async Task DownloadTracksIfConnected()
    {
        var ip = State.NetworkInfo.IpV4Address;
        
        if (ip == null || ip.Equals(IPAddress.Parse("0.0.0.0")) || !State.NetworkInfo.Connected)
            return;

        try
        {
            using var client = new HttpClient();
            Slots = await client.GetFromJsonAsync<List<SlotInfo>>($"http://{ip}/api/slots");
            SetTrackAndArtistName();
        }
        catch (HttpRequestException e)
        {
            // unable to download slots - keep null
            Slots = null;
        }
    }

    public async Task DisconnectAndUnsubscribe()
    {
        RemoveUpdateEventHandlers();

        var bleDevice = _device.DeviceRef;
        await _adapter.DisconnectDeviceAsync(bleDevice);
    }
    
    public void Dispose()
    {
        _adapter.DeviceDisconnected -= AdapterOnDeviceDisconnected;
        _adapter.DisconnectDeviceAsync(_device.DeviceRef);
        RemoveUpdateEventHandlers();
    }

    private async Task ReadAndSubscribeCharacteristic(IService service, Guid id, Action<byte[]> updatedHandler)
    {
        var characteristic = await service.GetCharacteristicAsync(id);
        if (characteristic == null)
            throw new InvalidOperationException($"Characteristic {id} not found");

        var initialValue = await characteristic.ReadAsync();
        updatedHandler(initialValue.data);

        EventHandler<CharacteristicUpdatedEventArgs> eventSubscription = (_, e) => updatedHandler(e.Characteristic.Value);
        characteristic.ValueUpdated += eventSubscription;
        await characteristic.StartUpdatesAsync();
        
        _subscribedCharacteristics.Add((characteristic, eventSubscription));
    }

    private void RemoveUpdateEventHandlers()
    {
        foreach (var (characteristic, handler) in _subscribedCharacteristics)
            characteristic.ValueUpdated -= handler;
        
        _subscribedCharacteristics.Clear();
    }

    private void AdapterOnDeviceDisconnected(object? sender, DeviceEventArgs e)
    {
        if (e.Device.Id != _device.DeviceRef.Id)
            return;

        RemoveUpdateEventHandlers();

        Disconnected?.Invoke(this, EventArgs.Empty);
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
        Console.WriteLine(string.Join(",", bytes.Select(b => $"{b:X2}")));
        
        State.PlayingInfo.State = playerStatePayload.State;
        State.PlayingInfo.SlotActive = playerStatePayload.SlotActive;
        State.PlayingInfo.FileIndex = playerStatePayload.FileIndex;
        State.PlayingInfo.FileCount = playerStatePayload.FileCount;
        State.PlayingInfo.CurrentTime = playerStatePayload.CurrentTime;
        State.PlayingInfo.Duration = playerStatePayload.Duration;
        State.PlayingInfo.Volume = playerStatePayload.Volume;
        State.PlayingInfo.MaxVolume = playerStatePayload.MaxVolume;

        SetTrackAndArtistName();
    }

    private void SetTrackAndArtistName()
    {
        State.PlayingInfo.CurrentTrackTitle = "--";
        State.PlayingInfo.CurrentTrackArtist = "";
        
        if (Slots == null)
            return;
        
        var currentSlot = State.PlayingInfo.SlotActive;
        var currentTrackIndex = State.PlayingInfo.FileIndex;
        
        if(Slots.Count <= currentSlot)
            return;
    
        if(Slots[currentSlot].Files.Count <= currentTrackIndex)
            return;
    
        var trackInfo = Slots[currentSlot].Files[currentTrackIndex];

        if (!string.IsNullOrEmpty(trackInfo.Artist))
            State.PlayingInfo.CurrentTrackArtist = trackInfo.Artist;
            
        if (!string.IsNullOrEmpty(trackInfo.Title))
            State.PlayingInfo.CurrentTrackTitle = trackInfo.Title;
        else if(trackInfo.Path != null)
            State.PlayingInfo.CurrentTrackTitle = Path.GetFileName(trackInfo.Path);
    }

    private void DeserializeUpdateNetworkValues(byte[]? bytes)
    {
        if(bytes == null || bytes.Length == 0)
            return;

        var networkStatePayload = NetworkStateCharacteristic.Parser.ParseFrom(bytes);
        State.NetworkInfo.Connected = networkStatePayload.Connected;
        State.NetworkInfo.Enabled = networkStatePayload.Enabled;
        State.NetworkInfo.IpV4Address = GetIpAddressFromInt(networkStatePayload.IpV4Address);
        State.NetworkInfo.Rssi = networkStatePayload.Rssi;
    }

    private static IPAddress GetIpAddressFromInt(int ipAddress)
    {
        var bytes = BitConverter.GetBytes(ipAddress);
        return new IPAddress(bytes);
    }
}