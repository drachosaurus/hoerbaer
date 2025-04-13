using System.Collections.ObjectModel;
using BaerControlApp.Comm;
using CommunityToolkit.Mvvm.Input;

namespace BaerControlApp;

public class MainViewModel: ViewModel
{
    private readonly BearDiscovery _bearDiscovery;

    public string Title => _bearDiscovery.BluetoothAvailable ? "Scanning..." : "Bluetooth not available!";
    public bool BluetoothAvailable => _bearDiscovery.BluetoothAvailable;
    public ObservableCollection<DiscoveredDevice> Devices => _bearDiscovery.Devices;

    public IAsyncRelayCommand NavigateDetailsCommand { get; }
    
    public MainViewModel(BearDiscovery bearDiscovery)
    {
        _bearDiscovery = bearDiscovery;
        _bearDiscovery.BluetoothStateChanged += BearBluetoothOnBluetoothStateChanged;

        NavigateDetailsCommand = new AsyncRelayCommand<DiscoveredDevice>(async device => 
            await Shell.Current.GoToAsync($"/device", new ShellNavigationQueryParameters { { "device", device! } }));
        
        Task.Run(() => _bearDiscovery.Discover(CancellationToken.None));
    }

    private void BearBluetoothOnBluetoothStateChanged(object? sender, EventArgs e)
    {
        OnPropertyChanged(nameof(Title));
        OnPropertyChanged(nameof(BluetoothAvailable));
    }
}