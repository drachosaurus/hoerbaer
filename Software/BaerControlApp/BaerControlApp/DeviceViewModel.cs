using BaerControlApp.Comm;
using CommunityToolkit.Mvvm.ComponentModel;
using Plugin.BLE.Abstractions.Exceptions;

namespace BaerControlApp;

public class DeviceViewModel : ObservableObject
{
    private readonly INotificationHub _notificationHub;
    private readonly BearDiscovery _discovery;
    private BearConnection _connection;

    public string BatteryVoltageDisplay => $"{_connection?.State.Power.BatteryVoltage:0.00}V";
    public string BatteryPercentageDisplay => $"{_connection?.State.Power.BatteryPercentage:0.00}%";
    public string BatteryChargingDisplay => (_connection?.State.Power.Charging ?? false) ? "Charging" : "Not Charging";
    
    public DeviceViewModel(INotificationHub notificationHub)
    {
        _notificationHub = notificationHub;
    }

    public async Task Initialize(DiscoveredDevice device)
    {
        try
        {
            _connection = new BearConnection(device);
            await _connection.ConnectAndSubscribe();
            
            _connection.State.Power.PropertyChanged += (sender, args) =>
            {
                OnPropertyChanged(nameof(BatteryVoltageDisplay));
                OnPropertyChanged(nameof(BatteryPercentageDisplay));
                OnPropertyChanged(nameof(BatteryChargingDisplay));
            };
        }
        catch(DeviceConnectionException ex)
        {
            await _notificationHub.ShowMessage("Unable to connect.");
            await Shell.Current.GoToAsync("main");
        }
        catch(Exception ex)
        {
            await _notificationHub.ShowException("Unexpected error while connecting.", ex);
            await Shell.Current.GoToAsync("main");
        }
    }
}