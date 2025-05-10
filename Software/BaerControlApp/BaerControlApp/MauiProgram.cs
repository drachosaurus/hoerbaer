using BaerControlApp.Comm;
using BaerControlApp.Device;
using Microsoft.Extensions.Logging;

namespace BaerControlApp;

public static class MauiProgram
{
    public static MauiApp CreateMauiApp()
    {
        var builder = MauiApp.CreateBuilder();
        builder
            .UseMauiApp<App>()
            .ConfigureFonts(fonts =>
            {
                fonts.AddFont("OpenSans-Regular.ttf", "OpenSansRegular");
                fonts.AddFont("OpenSans-Semibold.ttf", "OpenSansSemibold");
            });
        
        builder.Services.AddSingleton<AppShell>();
        builder.Services.AddSingleton<INotificationHub>(r => r.GetRequiredService<AppShell>());
        
        builder.Services.AddSingleton<BearConnectionManager>();
        builder.Services.AddSingleton<MainViewModel>();
        builder.Services.AddScoped<DeviceViewModel>();

#if DEBUG
        builder.Logging.AddDebug();
#endif

        return builder.Build();
    }
}