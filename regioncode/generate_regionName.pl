#!/usr/bin/perl

# Used to generate regionName.c
# usage: ./generate_regionName.pl > ../libGeoIP/regionName.c

use strict;

#print qq(#include "regionname.h"\n);
print qq(#include <string.h>\n);
print qq(#include <stdio.h>\n);

print "const char * GeoIP_region_name_by_code(const char * country_code,const char * region_code) {\n";
print "  const char * name = NULL;\n";
print "  int region_code2 = -1;\n";
print "  if (region_code == NULL) { return NULL; }\n";
print "  if ((region_code[0] >= 65) & (region_code[0] < (65 + 26))) {\n";
print "    if ((region_code[1] >= 65) & (region_code[1] < (65 + 26))) {\n";
print "      if ((strcmp(country_code,".qq("US").") == 0) | \n";
print "        (strcmp(country_code,".qq("CA").") == 0)) {\n";
print "          region_code2 = ((region_code[0]-65)*26)+(region_code[1]-65);\n";
print "      }\n";
print "    }\n";
print "    if ((region_code[1] >= 48) & (region_code[1] < (48 + 10))) {\n";
print "      region_code2 = ((region_code[0]-65)*10)+(region_code[1]-48)+100;\n";
print "    }\n";
print "  }\n";
print "  if ((region_code[0] >= 48) & (region_code[0] < (48 + 10))) {\n";
print "    if ((region_code[1] >= 48) & (region_code[1] < (48 + 10))) {\n";
print "      region_code2 = ((region_code[0]-48)*10)+(region_code[1]-48);\n";
print "    }\n";
print "  }\n";

print "  if (region_code2 == -1) {return NULL;}\n";

# iso3166_2.txt extracted from http://www.maxmind.com/app/iso3166_2
open(FILE,"iso3166_2.txt");
<FILE>;
my $last_country_code = "";
while (my $str = <FILE>){
  chomp($str);
  my ($country_code,$region_code,$name) = split(",",$str);
  my $region_code2 = ((ord(substr($region_code,0,1))-65)*26)+
  ord(substr($region_code,1,1))-65;
  readcode($last_country_code,$country_code,$region_code,$region_code2,$name);
  $last_country_code = $country_code;
}
close(FILE);
print "    }\n";
print "  }\n";

# fips10_4.txt extracted from http://www.maxmind.com/app/fips10_4
open(FILE,"fips10_4.txt");
<FILE>;
$last_country_code = "";
while (my $str = <FILE>){
  chomp($str);
  my ($country_code,$region_code,$name) = split(",",$str);
  if ($country_code eq "US") {next;}
  if ($country_code eq "CA") {next;}
  my $region_code2;
  if ($region_code =~ m![A-Z][0-9]!s){
    $region_code2 = ((ord(substr($region_code,0,1))-65)*10)+
    substr($region_code,1,1)+100;
  }else{
    $region_code2 = $region_code;
    $region_code2 =~ s!^0+!!g;
    $region_code2 =~ s!\s+!!g;
  }
  readcode($last_country_code,$country_code,$region_code,$region_code2,$name);
  $last_country_code = $country_code;
}
print "    }\n";
print "  }\n";
print "  return name;\n";
print "}\n";

close(FILE);

sub readcode {
  my ($last_country_code,$country_code,$region_code,$region_code2,$name) = @_;
  if ($country_code ne $last_country_code){
    if ($last_country_code ne ""){
      print "    }\n";
      print "  }\n";
    }
    print "  if (strcmp(country_code," . qq(") . $country_code . qq(") .  ") == 0) {\n";
    print "    switch (region_code2) {\n";
  }
#  $name =~ s!\s+!!g;
  $name =~ s!\"!!g;
  $name = qq(") . $name . qq(");
  print "      case " . $region_code2 . ":\n";
  print "      name = " . $name . ";\n";
  print "      break;\n";
}
