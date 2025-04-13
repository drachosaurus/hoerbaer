using System.Collections.ObjectModel;
using Plugin.BLE;
using Plugin.BLE.Abstractions;
using Plugin.BLE.Abstractions.Contracts;

namespace BaerControlApp.Comm;

public class BearDiscovery
{
    private readonly IAdapter _adapter;
    private readonly IBluetoothLE _ble;

    private BluetoothState _currentState;

    public event EventHandler BluetoothStateChanged;
    public bool BluetoothAvailable => _currentState == BluetoothState.On;

    public ObservableCollection<DiscoveredDevice> Devices { get; } = new();
    
    public BearDiscovery()
    {
        _ble = CrossBluetoothLE.Current;
        _adapter = CrossBluetoothLE.Current.Adapter;
        _adapter.ScanMode = ScanMode.Balanced;

        _ble.StateChanged += (s, e) =>
        {
            _currentState = e.NewState;
            BluetoothStateChanged?.Invoke(this, EventArgs.Empty);
            Console.WriteLine($"The bluetooth state changed to {e.NewState}");
        };
        
        _adapter.DeviceDiscovered += (s,a) => AddOrUpdateDevice(a.Device);
        _adapter.DeviceAdvertised += (s,a) => AddOrUpdateDevice(a.Device);
    }
    
    public async Task Discover(CancellationToken ct)
    {
        var scanFilterOptions = new ScanFilterOptions
        {
            ServiceUuids = [ KnownIds.ServiceId ]
        };
        
        await _adapter.StartScanningForDevicesAsync(cancellationToken: ct, scanFilterOptions: scanFilterOptions);
    }

    private void AddOrUpdateDevice(IDevice device)
    {
        var existing = Devices.SingleOrDefault(d => d.Id == device.Id);
        if (existing != null)
        {
            existing.Update(device);
            Console.WriteLine($"Updated device: {device.Name} ({device.Id})");
        }
        else
        {
            Devices.Add(new DiscoveredDevice(device));
            Console.WriteLine($"Added device: {device.Name} ({device.Id})");
        }
    }
}