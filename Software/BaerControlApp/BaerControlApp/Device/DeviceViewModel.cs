using BaerControlApp.Comm;
using CommunityToolkit.Mvvm.ComponentModel;
using HoerBaer.Ble;
using Plugin.BLE.Abstractions.Exceptions;

namespace BaerControlApp.Device;

public class DeviceViewModel : ObservableObject
{
    private readonly INotificationHub _notificationHub;
    private BearConnection? _connection;

    public string Name { get; private set; }
    
    public float? BatteryVoltage => _connection?.State.Power.BatteryVoltage ?? 0;
    public float? BatteryPercentage => _connection?.State.Power.BatteryPercentage ?? 0;
    public bool IsCharging => _connection?.State.Power.Charging ?? false;
    
    public string StateDisplay =>
        _connection != null ? 
            HumanizeState(_connection.State.PlayingInfo.State) : "--";

    public string SlotDisplay => _connection != null ? 
        $"{_connection.State.PlayingInfo.SlotActive + 1}" : "--";

    public string FileDisplay => _connection != null ? 
        $"{_connection.State.PlayingInfo.FileIndex + 1}/{_connection.State.PlayingInfo.FileCount}" : "--/--";

    public string TimeDisplay => _connection != null ? 
        $"{TimeSpan.FromSeconds(_connection.State.PlayingInfo.CurrentTime):mm\\:ss} / {TimeSpan.FromSeconds(_connection.State.PlayingInfo.Duration):mm\\:ss}" : "--:-- / --:--";

    public string VolumeDisplay => _connection != null ? 
        $"{_connection.State.PlayingInfo.Volume} / {_connection.State.PlayingInfo.MaxVolume}" : "--/--";
    
    public DeviceViewModel(INotificationHub notificationHub)
    {
        _notificationHub = notificationHub;
    }

    public async Task Initialize(DiscoveredDevice device)
    {
        try
        {
            Name = device.Name;
            OnPropertyChanged(nameof(Name));
            
            _connection = new BearConnection(device);
            await _connection.ConnectAndSubscribe();

            _connection.State.Power.PropertyChanged += (sender, args) =>
            {
                OnPropertyChanged(nameof(BatteryVoltage));
                OnPropertyChanged(nameof(BatteryPercentage));
                OnPropertyChanged(nameof(IsCharging));
            };

            _connection.State.PlayingInfo.PropertyChanged += (sender, args) =>
            {
                OnPropertyChanged(nameof(StateDisplay));
                OnPropertyChanged(nameof(SlotDisplay));
                OnPropertyChanged(nameof(FileDisplay));
                OnPropertyChanged(nameof(TimeDisplay));
                OnPropertyChanged(nameof(VolumeDisplay));
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
    
    private string HumanizeState(PlayerState playingInfoState)
        => playingInfoState switch
        {
            PlayerState.PlayerStopped => "Stopped",
            PlayerState.PlayerPlaying => "Playing",
            PlayerState.PlayerPaused => "Pause",
            _ => "--"
        };
}