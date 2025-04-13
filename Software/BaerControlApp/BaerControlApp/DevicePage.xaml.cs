using BaerControlApp.Comm;

namespace BaerControlApp;

[QueryProperty(nameof(Device), "device")]
public partial class DevicePage : ContentPage
{
    private readonly IServiceScopeFactory _serviceScopeFactory;
    
    private IServiceScope? _currentScope;
    
    DiscoveredDevice _device;
    public DiscoveredDevice Device
    {
        get => _device;
        set
        {
            if (_device?.Id == value.Id)
                return;
            
            _device = value;
            
            _currentScope?.Dispose();
            _currentScope = _serviceScopeFactory.CreateScope();
            var viewModel = _currentScope.ServiceProvider.GetRequiredService<DeviceViewModel>();
            BindingContext = viewModel;
            _ = viewModel.Initialize(_device);
        }
    }
    
    public DevicePage(IServiceScopeFactory serviceScopeFactory)
    {
        InitializeComponent();
        _serviceScopeFactory = serviceScopeFactory;
    }
}