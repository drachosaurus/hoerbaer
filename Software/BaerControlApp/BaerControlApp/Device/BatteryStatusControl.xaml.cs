using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BaerControlApp.Device;

public partial class BatteryStatusControl : ContentView
{
    public static readonly BindableProperty BatteryVoltageProperty =
        BindableProperty.Create(nameof(BatteryVoltage), typeof(float?), typeof(BatteryStatusControl), null,
            propertyChanged: OnBatteryVoltageChanged);

    public static readonly BindableProperty BatteryPercentageProperty =
        BindableProperty.Create(nameof(BatteryPercentage), typeof(float?), typeof(BatteryStatusControl), null,
            propertyChanged: OnBatteryPercentageChanged);

    public static readonly BindableProperty IsChargingProperty =
        BindableProperty.Create(nameof(IsCharging), typeof(bool), typeof(BatteryStatusControl), false);

    public float? BatteryVoltage
    {
        get => (float?)GetValue(BatteryVoltageProperty);
        set => SetValue(BatteryVoltageProperty, value);
    }

    public float? BatteryPercentage
    {
        get => (float?)GetValue(BatteryPercentageProperty);
        set => SetValue(BatteryPercentageProperty, value);
    }

    public bool IsCharging
    {
        get => (bool)GetValue(IsChargingProperty);
        set => SetValue(IsChargingProperty, value);
    }

    // Display properties - formatted strings based on the numeric values
    public string BatteryVoltageDisplay => BatteryVoltage.HasValue ? $"{BatteryVoltage:F1}V" : "N/A";
    public string BatteryPercentageDisplay => BatteryPercentage.HasValue ? $"{BatteryPercentage:F0}%" : "N/A";

    // Used for color calculations and fill display
    public Color BatteryFillColor => GetBatteryColor();
    public double BatteryPercentageRatio => BatteryPercentage.HasValue ? Math.Clamp(BatteryPercentage.Value / 100.0, 0, 1) : 0;

    public BatteryStatusControl()
    {
        InitializeComponent();
    }

    private static void OnBatteryVoltageChanged(BindableObject bindable, object oldValue, object newValue)
    {
        if (bindable is BatteryStatusControl control)
        {
            control.OnPropertyChanged(nameof(BatteryVoltageDisplay));
        }
    }

    private static void OnBatteryPercentageChanged(BindableObject bindable, object oldValue, object newValue)
    {
        if (bindable is BatteryStatusControl control)
        {
            control.OnPropertyChanged(nameof(BatteryPercentageDisplay));
            control.OnPropertyChanged(nameof(BatteryPercentageRatio));
            control.OnPropertyChanged(nameof(BatteryFillColor));
        }
    }

    private Color GetBatteryColor()
    {
        if (!BatteryPercentage.HasValue)
            return Colors.Gray;

        // Red for low battery, yellow for medium, green for good
        float percentage = BatteryPercentage.Value;
        if (percentage <= 20)
            return Colors.Red;
        else if (percentage <= 50)
            return Colors.Orange;
        else
            return Colors.Green;
    }
}

// Converter to calculate battery fill width based on percentage
public class PercentageToWidthConverter : IValueConverter
{
    public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
    {
        if (value is double totalWidth && parameter is double percentage)
        {
            return totalWidth * percentage;
        }
        return 0;
    }

    public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}