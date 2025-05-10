using System.ComponentModel;
using BaerControlApp.Comm;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using HoerBaer.Ble;
using Plugin.BLE.Abstractions.Exceptions;

namespace BaerControlApp.Device;

public class DeviceViewModel : ObservableObject, IDisposable
{
    private readonly INotificationHub _notificationHub;
    private readonly BearConnectionManager _connectionManager;
    private BearConnection? _connection;

    public string Name { get; private set; }
    
    public float? BatteryVoltage => _connection?.State.Power.BatteryVoltage ?? 0;
    public float? BatteryPercentage => _connection?.State.Power.BatteryPercentage ?? 0;
    public bool IsCharging => _connection?.State.Power.Charging ?? false;
    
    public bool WifiIsConnected => _connection?.State.NetworkInfo.Connected ?? false;
    public bool WifiIsEnabled => _connection?.State.NetworkInfo.Enabled ?? false;
    public int WifiRssi => _connection?.State.NetworkInfo.Rssi ?? 0;
    
    public string StateDisplay =>
        _connection != null ? 
            HumanizeState(_connection.State.PlayingInfo.State) : "--";

    public string SlotDisplay => _connection != null ? 
        $"{_connection.State.PlayingInfo.SlotActive + 1}" : "--";
    
    public string TrackDisplay => _connection != null ? 
        GetCurrentTrackDisplay(_connection.State) : "--";
    
    public string FileIndexDisplay => _connection != null ? 
        $"{_connection.State.PlayingInfo.FileIndex + 1}/{_connection.State.PlayingInfo.FileCount}" : "--/--";

    public string TimeDisplay => _connection != null ? 
        $"{TimeSpan.FromSeconds(_connection.State.PlayingInfo.CurrentTime):mm\\:ss} / {TimeSpan.FromSeconds(_connection.State.PlayingInfo.Duration):mm\\:ss}" : "--:-- / --:--";

    public string VolumeDisplay => _connection != null ? 
        $"{_connection.State.PlayingInfo.Volume} / {_connection.State.PlayingInfo.MaxVolume}" : "--/--";
    
    public IAsyncRelayCommand NavigateTracklistCommand { get; }
    
    public DeviceViewModel(INotificationHub notificationHub, BearConnectionManager connectionManager)
    {
        _notificationHub = notificationHub;
        _connectionManager = connectionManager;
        Name = "";
        
        NavigateTracklistCommand = new AsyncRelayCommand<DiscoveredDevice>(async device => 
            await Shell.Current.GoToAsync($"//device/tracklist", new ShellNavigationQueryParameters { { "connection", _connection! } }));
    }

    public async Task Initialize(DiscoveredDevice device)
    {
        try
        {
            Name = device.Name;
            OnPropertyChanged(nameof(Name));

            _connection = await _connectionManager.GetConnectedBaer(device);
            OnPowerOnPropertyChanged(null, null);
            OnPlayingInfoOnPropertyChanged(null, null);
            OnNetworkInfoOnPropertyChanged(null, null);
            
            _connection.State.Power.PropertyChanged += OnPowerOnPropertyChanged;
            _connection.State.PlayingInfo.PropertyChanged += OnPlayingInfoOnPropertyChanged;
            _connection.State.NetworkInfo.PropertyChanged += OnNetworkInfoOnPropertyChanged;
            
            _connection.Disconnected += ConnectionOnDisconnected;
        }
        catch(DeviceConnectionException ex)
        {
            await _notificationHub.ShowMessage("Unable to connect.");
            await Shell.Current.GoToAsync("//main");
        }
        catch(Exception ex)
        {
            await _notificationHub.ShowException("Unexpected error while connecting.", ex);
            await Shell.Current.GoToAsync("//main");
        }
    }

    private async void ConnectionOnDisconnected(object? sender, EventArgs e)
    {
        var dispatcher = Dispatcher.GetForCurrentThread();
        if (dispatcher == null)
            return;

        var state = Shell.Current.CurrentState;
        
        await dispatcher.DispatchAsync(async () =>
        {
            await _notificationHub.ShowMessage("Device disconnected.");
            await Shell.Current.GoToAsync("//main");
        });
    }

    private void OnNetworkInfoOnPropertyChanged(object? sender, PropertyChangedEventArgs? args)
    {
        OnPropertyChanged(nameof(WifiIsConnected));
        OnPropertyChanged(nameof(WifiIsEnabled));
        OnPropertyChanged(nameof(WifiRssi));
    }

    private void OnPlayingInfoOnPropertyChanged(object? sender, PropertyChangedEventArgs? args)
    {
        OnPropertyChanged(nameof(StateDisplay));
        OnPropertyChanged(nameof(TrackDisplay));
        OnPropertyChanged(nameof(SlotDisplay));
        OnPropertyChanged(nameof(FileIndexDisplay));
        OnPropertyChanged(nameof(TimeDisplay));
        OnPropertyChanged(nameof(VolumeDisplay));
    }

    private void OnPowerOnPropertyChanged(object? sender, PropertyChangedEventArgs? args)
    {
        OnPropertyChanged(nameof(BatteryVoltage));
        OnPropertyChanged(nameof(BatteryPercentage));
        OnPropertyChanged(nameof(IsCharging));
    }

    private string GetCurrentTrackDisplay(BearState state)
    {
        if (!(_connection?.Slots?.Any() ?? false))
            return "--";

        var currentSlot = _connection.State.PlayingInfo.SlotActive;
        var currentTrackIndex = _connection.State.PlayingInfo.FileIndex;

        if(_connection.Slots.Count <= currentSlot)
            return "--";
        
        if(_connection.Slots[currentSlot].Files.Count <= currentTrackIndex)
            return "--";
        
        var trackInfo = _connection.Slots[currentSlot].Files[currentTrackIndex];
        
        if(trackInfo.Title != null || trackInfo.Artist != null)
            return $"{trackInfo.Title} - {trackInfo.Artist}";
        
        if(trackInfo.Title != null)
            return trackInfo.Title;

        return trackInfo.Path ?? "--";
    }

    private static string HumanizeState(PlayerState playingInfoState)
        => playingInfoState switch
        {
            PlayerState.PlayerStopped => "Stopped",
            PlayerState.PlayerPlaying => "Playing",
            PlayerState.PlayerPaused => "Pause",
            _ => "--"
        };

    public void Dispose()
    {
        if (_connection == null)
            return;
        
        _connection.State.Power.PropertyChanged -= OnPowerOnPropertyChanged;
        _connection.State.PlayingInfo.PropertyChanged -= OnPlayingInfoOnPropertyChanged;
        _connection.State.NetworkInfo.PropertyChanged -= OnNetworkInfoOnPropertyChanged;
            
        _connection.Disconnected -= ConnectionOnDisconnected;
    }
}