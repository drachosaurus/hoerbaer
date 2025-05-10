using CommunityToolkit.Mvvm.ComponentModel;
using Plugin.BLE.Abstractions.Contracts;

namespace BaerControlApp.Comm;

public class DiscoveredDevice : ObservableObject
{
    private string _name;
    public string Name
    {
        get => _name;
        set => SetProperty(ref _name, value);
    }

    private Guid _id;
    public Guid Id
    {
        get => _id;
        set => SetProperty(ref _id, value);
    }

    private int _rssi;
    public int Rssi
    {
        get => _rssi;
        set => SetProperty(ref _rssi, value);
    }
    
    public bool IsConnected => DeviceRef?.State == Plugin.BLE.Abstractions.DeviceState.Connected;
    
    public IDevice DeviceRef { get; }

    public DiscoveredDevice(IDevice device)
    {
        DeviceRef = device;
        Update(device);
    }

    public void Update(IDevice device)
    {
        Name = device.Name;
        Id = device.Id;
        Rssi = device.Rssi;
    }
}