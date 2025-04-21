namespace BaerControlApp.Device;

public partial class WifiStatusView : ContentView
{
    public static readonly BindableProperty WifiIsConnectedProperty =
        BindableProperty.Create(nameof(WifiIsConnected), typeof(bool), typeof(WifiStatusView), false, 
            propertyChanged: OnWifiStatusChanged);
        
    public static readonly BindableProperty WifiIsEnabledProperty =
        BindableProperty.Create(nameof(WifiIsEnabled), typeof(bool), typeof(WifiStatusView), false,
            propertyChanged: OnWifiStatusChanged);
        
    public static readonly BindableProperty WifiRssiProperty =
        BindableProperty.Create(nameof(WifiRssi), typeof(int), typeof(WifiStatusView), 0,
            propertyChanged: OnWifiStatusChanged);

    public bool WifiIsConnected
    {
        get => (bool)GetValue(WifiIsConnectedProperty);
        set => SetValue(WifiIsConnectedProperty, value);
    }

    public bool WifiIsEnabled
    {
        get => (bool)GetValue(WifiIsEnabledProperty);
        set => SetValue(WifiIsEnabledProperty, value);
    }

    public int WifiRssi
    {
        get => (int)GetValue(WifiRssiProperty);
        set => SetValue(WifiRssiProperty, value);
    }

    public WifiStatusView()
    {
        InitializeComponent();
        
        UpdateStatus();
    }

    private static void OnWifiStatusChanged(BindableObject bindable, object oldValue, object newValue)
    {
        if (bindable is WifiStatusView wifiStatusView)
        {
            wifiStatusView.UpdateStatus();
        }
    }

    private void UpdateStatus()
    {
        if (!WifiIsEnabled)
        {
            StatusLabel.Text = "WiFi Disabled";
            return;
        }

        if (WifiIsConnected)
        {
            StatusLabel.Text = "Connected";
        }
        else
        {
            StatusLabel.Text = "Disconnected";
        }
    }
}

public class BoolToColorConverter : IValueConverter
{
    public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
    {
        if (value is bool boolValue && parameter is string colors)
        {
            string[] colorValues = colors.Split(',');
            if (colorValues.Length >= 2)
            {
                return Color.FromArgb(boolValue ? colorValues[0] : colorValues[1]);
            }
        }
        return Colors.Gray;
    }

    public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}

public class RssiToColorConverter : IValueConverter
{
    public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
    {
        if (value is int rssi && parameter is string barIndexStr && int.TryParse(barIndexStr, out int barIndex))
        {
            // RSSI values typically range from -100 (very weak) to -30 (very strong)
            // Normalize RSSI to 0-100 scale
            int signalStrength = Math.Max(0, Math.Min(100, 2 * (rssi + 100)));
            
            // Determine which bars should be active based on signal strength
            bool isActive = false;
            
            switch (barIndex)
            {
                case 1: // First bar active if signal > 0
                    isActive = signalStrength > 0;
                    break;
                case 2: // Second bar active if signal > 33
                    isActive = signalStrength > 33;
                    break;
                case 3: // Third bar active if signal > 66
                    isActive = signalStrength > 66;
                    break;
            }

            return isActive ? Colors.DodgerBlue : Colors.Gray;
        }
        
        return Colors.Gray;
    }

    public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}

