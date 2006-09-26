#include <stdio.h>
#include <GeoIP.h>
#include <GeoIPCity.h>
#include <sys/time.h>


char *ipstring[4] = {"24.24.24.24","80.24.24.80",
"200.24.24.40","68.24.24.46"};
int numipstrings = 4;
struct timeval timer_t1;
struct timeval timer_t2;


void timerstart(){
  gettimeofday(&timer_t1,NULL);
}
double timerstop(){
  gettimeofday(&timer_t2,NULL);
  int a1 = timer_t2.tv_sec - timer_t1.tv_sec;
  int a2 = timer_t2.tv_usec - timer_t1.tv_usec;
  if (a1 < 0){
    a1 = a1 - 1;
    a2 = a2 + 1000000;
  }
  double r = (((double) a1) + (((double) a2) / 1000000));
  return r;
}

void testgeoipcountry(int flags,const char *msg,int numlookups){
  GeoIP *i = GeoIP_open("/usr/local/share/GeoIP/GeoIP.dat",flags);
  if (i == NULL){
    printf("error: GeoIP.dat does not exist\n");
    return;
  }
  const char *str;
  timerstart();
  int i4 = 0;
  int i2 = 0;
  for (i2 = 0;i2 < numlookups;i2++){
    str = GeoIP_country_name_by_addr(i,ipstring[i4]);
    i4 = (i4 + 1) % numipstrings;
  }
  double t = timerstop();
  printf("%s\n", msg);
  printf("%d lookups made in %f seconds \n",numlookups,t);
  GeoIP_delete(i);
}

void testgeoipregion(int flags,const char *msg,int numlookups){
  GeoIP *i = GeoIP_open("/usr/local/share/GeoIP/GeoIPRegion.dat",flags);
  if (i == NULL){
    printf("error: GeoIPRegion.dat does not exist\n");
    return;
  }
  GeoIPRegion *i3;
  timerstart();
  int i4 = 0;
  int i2 = 0;
  for (i2 = 0;i2 < numlookups;i2++){
    i3 = GeoIP_region_by_addr(i,ipstring[i4]);
    GeoIPRegion_delete(i3); 
    i4 = (i4 + 1) % numipstrings;
  }
  double t = timerstop();
  printf("%s\n", msg);
  printf("%d lookups made in %f seconds \n",numlookups,t);
  GeoIP_delete(i);
}

void testgeoipcity(int flags,const char *msg,int numlookups){
  GeoIP *i = GeoIP_open("/usr/local/share/GeoIP/GeoIPCity.dat",flags);
  if (i == NULL){
    printf("error: GeoLiteCity.dat does not exist\n");
    return;
  }
  GeoIPRecord * i3;
  timerstart();
  int i4 = 0;
  int i2 = 0;
  for (i2 = 0;i2 < numlookups;i2++){
    i3 = GeoIP_record_by_addr(i,ipstring[i4]);
    GeoIPRecord_delete(i3);
    i4 = (i4 + 1) % numipstrings;
  }
  double t = timerstop();
  printf("%s\n", msg);
  printf("%d lookups made in %f seconds \n",numlookups,t);
  GeoIP_delete(i);
}

int main(){
  int time = 300*numipstrings;
  testgeoipcountry(0,"GeoIP Country",100*time);
  testgeoipcountry(GEOIP_CHECK_CACHE,"GeoIP Country with GEOIP_CHECK_CACHE",100*time);
  testgeoipcountry(GEOIP_MEMORY_CACHE,"GeoIP Country with GEOIP_MEMORY_CACHE",1000*time);
  testgeoipcountry(GEOIP_MEMORY_CACHE | GEOIP_CHECK_CACHE,"GeoIP Country with GEOIP_MEMORY_CACHE and GEOIP_CHECK_CACHE",1000*time);

  testgeoipregion(0,"GeoIP Region",100*time);
  testgeoipregion(GEOIP_CHECK_CACHE,"GeoIP Region with GEOIP_CHECK_CACHE",100*time);
  testgeoipregion(GEOIP_MEMORY_CACHE,"GeoIP Region with GEOIP_MEMORY_CACHE",1000*time);
  testgeoipregion(GEOIP_MEMORY_CACHE | GEOIP_CHECK_CACHE,"GeoIP Region with GEOIP_MEMORY_CACHE and GEOIP_CHECK_CACHE",1000*time);

  testgeoipcity(0,"GeoIP City",50*time);
  testgeoipcity(GEOIP_INDEX_CACHE,"GeoIP City with GEOIP_INDEX_CACHE",200*time);
  testgeoipcity(GEOIP_INDEX_CACHE | GEOIP_CHECK_CACHE,"GeoIP City with GEOIP_INDEX_CACHE and GEOIP_CHECK_CACHE",200*time);
  testgeoipcity(GEOIP_MEMORY_CACHE,"GeoIP City with GEOIP_MEMORY_CACHE",500*time);
  return 0;
}
