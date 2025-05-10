using System.Collections.ObjectModel;
using BaerControlApp.Comm;
using CommunityToolkit.Mvvm.Input;

namespace BaerControlApp;

public class MainViewModel: ViewModel
{
    private readonly BearConnectionManager _bearConnectionManager;

    public string Title => _bearConnectionManager.BluetoothAvailable ? "Scanning..." : "Bluetooth not available!";
    public bool BluetoothAvailable => _bearConnectionManager.BluetoothAvailable;
    public ObservableCollection<DiscoveredDevice> Devices => _bearConnectionManager.Devices;

    public IAsyncRelayCommand NavigateDetailsCommand { get; }
    
    public MainViewModel(BearConnectionManager bearConnectionManager)
    {
        _bearConnectionManager = bearConnectionManager;
        _bearConnectionManager.BluetoothStateChanged += BearBluetoothOnBluetoothStateChanged;

        NavigateDetailsCommand = new AsyncRelayCommand<DiscoveredDevice>(async device => 
            await Shell.Current.GoToAsync($"device?deviceId={device!.Id}"));
        
        Task.Run(() => _bearConnectionManager.Discover(CancellationToken.None));
    }

    private void BearBluetoothOnBluetoothStateChanged(object? sender, EventArgs e)
    {
        OnPropertyChanged(nameof(Title));
        OnPropertyChanged(nameof(BluetoothAvailable));
    }

    public void CheckDiscovering()
    {
        if(!_bearConnectionManager.IsScanning)
            Task.Run(() => _bearConnectionManager.Discover(CancellationToken.None));
    }
}