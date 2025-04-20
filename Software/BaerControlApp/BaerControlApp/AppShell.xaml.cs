using BaerControlApp.Device;

namespace BaerControlApp;

public partial class AppShell : Shell, INotificationHub
{
    public AppShell()
    {
        InitializeComponent();
        
        Routing.RegisterRoute("main", typeof(MainPage));
        Routing.RegisterRoute("device", typeof(DevicePage));
    }

    public async Task ShowMessage(string message)
    {
        await DisplayAlert("Message", message, "OK");
    }

    public async Task ShowException(string message, Exception e)
    {
        var fullMessage = 
            message + Environment.NewLine +
            $"{e.GetType().Name}: {e.Message}" + Environment.NewLine +
            $"{e.StackTrace}";

        await DisplayAlert("Error!", fullMessage, "OK");
    }
}