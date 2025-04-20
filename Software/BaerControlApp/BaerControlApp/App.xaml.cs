using BaerControlApp.Comm;

namespace BaerControlApp;

public partial class App : Application
{
    private readonly IServiceProvider _serviceProvider;

    public App(IServiceProvider serviceProvider)
    {
        _serviceProvider = serviceProvider;

        InitializeComponent();
    }

    protected override Window CreateWindow(IActivationState? activationState)
    {
        var shell = _serviceProvider.GetRequiredService<AppShell>();
        var window = new Window(shell);

#if MACCATALYST
        window.MinimumWidth = 500;
        window.MaximumWidth = 500;
        window.MinimumHeight = 800;
        window.MaximumHeight = 800;
#endif
        
        return window;
    }
}