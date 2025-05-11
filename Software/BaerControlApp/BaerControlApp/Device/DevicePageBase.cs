using BaerControlApp.Comm;

namespace BaerControlApp.Device;

public class DevicePageBase<TViewModel> : ContentPage
    where TViewModel : IDeviceViewModel
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
            var viewModel = _currentScope.ServiceProvider.GetRequiredService<TViewModel>();
            BindingContext = viewModel;
            
            var mgr = _currentScope.ServiceProvider.GetRequiredService<BearConnectionManager>();
            var discoveredDevice = mgr.Devices.Single(d => d.Id == Guid.Parse(value));
            _ = viewModel.Initialize(discoveredDevice);
        }
    }

    public DevicePageBase(IServiceScopeFactory serviceScopeFactory)
    {
        _serviceScopeFactory = serviceScopeFactory;
    }

    protected override void OnDisappearing()
    {
        base.OnDisappearing();
        _currentScope?.Dispose();
        _currentScope = null;
    }
}