#include "Filter.h"
#include "chewing.h"
 
// the <float> makes a filter for float numbers
// 20 is the weight (20 => 20%)
// 0 is the initial value of the filter 

ExponentialFilter<double> FilteredTemperature(20, 0);        //SEE Above for description :)
ExponentialFilter<double> FilteredTemperature1(35, 0);        //SEE Above for description :)
ExponentialFilter<double> FilteredTemperature2(35, 0);        //SEE Above for description :)
 
double filter::chewing_it0(double raw_current){
  FilteredTemperature.Filter(raw_current);
  double SmoothTemperature = FilteredTemperature.Current();

  return SmoothTemperature;
}
double filter::chewing_it1(double raw_current){
  FilteredTemperature1.Filter(raw_current);
  double SmoothTemperature = FilteredTemperature1.Current();

  return SmoothTemperature;
}
double filter::chewing_it2(double raw_current){
  FilteredTemperature2.Filter(raw_current);
  double SmoothTemperature = FilteredTemperature2.Current();

  return SmoothTemperature;
}
