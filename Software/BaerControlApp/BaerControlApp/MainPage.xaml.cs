namespace BaerControlApp;

public partial class MainPage : ContentPage
{
    public MainPage(MainViewModel viewModel)
    {
        InitializeComponent();

        BindingContext = viewModel;
    }

    protected override void OnAppearing()
    {
        (BindingContext as MainViewModel)?.CheckDiscovering();
    }
}