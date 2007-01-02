#!/usr/bin/perl

use strict;

# Used to generate timeZone.c
# usage: ./generate_timeZone.pl > ../libGeoIP/timeZone.c

my $old_country;
my $old_region;
my $had_region;

# Obtain timezone.txt from http://www.maxmind.com/timezone.txt
open(FILE,"timezone.txt");
my $str = <FILE>;
print "#include <string.h> \n";
print "const char* GeoIP_time_zone_by_country_and_region(const char * country,const char * region) {\n";
print "  const char* timezone = NULL;\n";
print <<END;
  if (country == NULL) {
    return NULL;
  }
  if (region == NULL) {
    region = "";
  }
END

while ($str = <FILE>) {
  $str =~ s!\s*$!!; 
  my ($country,$region,$timezone) = split("\t",$str);
  if ($country ne $old_country) {
    if ($had_region) {
      print "    }\n";
      $had_region = "";
    }
    if ($old_country ne "") {
      print "  } else if (strcmp(country," . qq(") . $country . qq(") . ") == 0) {\n";
    } else {
      print "  if (strcmp(country," . qq(") . $country . qq(") . ") == 0) {\n";
    }
  }
  if ($region ne "") {
    $had_region = 1;
    if ($old_region ne "") {
      print "    } else if (strcmp(region," . qq(") . $region . qq(") . ") == 0) {\n  ";
    } else {
      print "    if (strcmp(region," . qq(") . $region . qq(") . ") == 0) {\n  ";
    }
  } elsif ($old_region ne "") {
    print "    } else {\n  ";
  }
  print qq(    timezone = ") . $timezone . qq(") . ";\n";
  $old_country = $country;
  $old_region = $region;
}
print "  }\n";
print "  return timezone;\n";
print "}\n";

close(FILE);
