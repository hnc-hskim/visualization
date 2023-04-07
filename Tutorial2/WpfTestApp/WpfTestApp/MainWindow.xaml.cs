using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace WpfTestApp
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private bool isDrawing;
        private Point start;
        private Polyline polyline;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void Canvas_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                isDrawing = true;
                start = e.GetPosition(canvas);
                polyline = new Polyline
                {
                    Stroke = Brushes.Black,
                    StrokeThickness = 30
                };

                // PathGeometry 생성
                PathGeometry pathGeometry = new PathGeometry();
                pathGeometry.Figures = new PathFigureCollection();
                pathGeometry.Figures.Add(new PathFigure()
                {
                    StartPoint = new Point(50, 50),
                    Segments = new PathSegmentCollection()
                    {
                        new LineSegment(new Point(150, 50), true),
                        new LineSegment(new Point(150, 150), true),
                        new LineSegment(new Point(50, 150), true)
                    }
                });

                // Pen 생성
                Pen pen = new Pen(Brushes.Red, 5);
                pen.LineJoin = PenLineJoin.Round;
                pen.StartLineCap = PenLineCap.Round; 
                pen.EndLineCap = PenLineCap.Round;

                polyline.Stroke = pen.Brush;
                canvas.Children.Add(polyline);
            }
        } 

        private void Canvas_MouseMove(object sender, MouseEventArgs e)
        {
            if (isDrawing)
            {
                Point current = e.GetPosition(canvas);
                polyline.Points.Add(current);
            }
        }

        private void Canvas_MouseUp(object sender, MouseButtonEventArgs e)
        {
            if (isDrawing)
            {
                isDrawing = false;
                polyline = null;
            }
        }
    }
}
