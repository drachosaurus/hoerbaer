namespace BaerControlApp;

public interface INotificationHub
{
    public Task ShowMessage(string message);
    
    public Task ShowException(string message, Exception e);
}