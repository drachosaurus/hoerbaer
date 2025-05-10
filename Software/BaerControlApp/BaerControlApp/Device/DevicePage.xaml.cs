using BaerControlApp.Comm;

namespace BaerControlApp.Device;

[QueryProperty(nameof(DeviceId), "deviceId")]
public partial class DevicePage : ContentPage
{
    private readonly IServiceScopeFactory _serviceScopeFactory;
    
    private IServiceScope? _currentScope;
    
    string _deviceId;
    public string DeviceId
    {
        get => _deviceId;
        set
        {
            if (_deviceId == value)
                return;
            
            _deviceId = value;
            
            _currentScope?.Dispose();
            _currentScope = _serviceScopeFactory.CreateScope();
            var viewModel = _currentScope.ServiceProvider.GetRequiredService<DeviceViewModel>();
            BindingContext = viewModel;
            
            var mgr = _currentScope.ServiceProvider.GetRequiredService<BearConnectionManager>();
            var discoveredDevice = mgr.Devices.Single(d => d.Id == Guid.Parse(value));
            _ = viewModel.Initialize(discoveredDevice);
        }
    }
    
    public DevicePage(IServiceScopeFactory serviceScopeFactory)
    {
        InitializeComponent();
        _serviceScopeFactory = serviceScopeFactory;
    }

    protected override void OnDisappearing()
    {
        base.OnDisappearing();
        _currentScope?.Dispose();
        _currentScope = null;
    }
}