#!/usr/bin/env perl

require "readvaluesfile.pl";

use Getopt::Std;
getopts('chspmi:');

# ARG 0 is properties.csv
%propmap  = read_properties_file($ARGV[0]);

# ARG 1 is value-types.txt
%valuemap  = read_values_file($ARGV[1]);


$include_vanew = 1;

# Write the file inline by copying everything before a demarcation
# line, and putting the generated data after the demarcation

if ($opt_i) {

  open(IN,$opt_i) || die "Can't open input file $opt_i";

  while(<IN>){

    if (/Do not edit/){
      last;
    }

    print;

  }    

    print "/* Everything below this line is machine generated. Do not edit. */\n";


}

sub fudge_data {
  my $prop = shift;

  my $value = $propmap{$prop}->{'lic_value'};

  if (!$value){
    die "Can't find value for property \"$prop\"\n";
  }
  my $ucf = join("",map {ucfirst(lc($_));}  split(/-/,$prop));
  my $lc = lc($ucf);
  my $uc = uc($lc);

  my $ucfvalue = join("",map {ucfirst(lc($_));}  split(/-/,$value));
  my $lcvalue = lc($ucfvalue);
  my $ucvalue = uc($lcvalue);

  my $type = $valuemap{$value}->{C}->[1];

  return ($uc,$lc,$lcvalue,$ucvalue,$type);

}  

# Create the property map data
if($opt_c){

  print "struct icalproperty_map property_map[] = {\n";
  
  foreach $prop (sort keys %propmap) {
    
    next if !$prop;
    
    next if $prop eq 'NO';
    
    my ($uc,$lc,$lcvalue,$ucvalue,$type) = fudge_data($prop);
    
    print "{ICAL_${uc}_PROPERTY,\"$prop\",ICAL_${ucvalue}_VALUE},\n";
    
  }
  
  $prop = "NO";
  
  my ($uc,$lc,$lcvalue,$ucvalue,$type) = fudge_data($prop);
  
  print "{ICAL_${uc}_PROPERTY,\"\",ICAL_NO_VALUE}};\n\n";


  print "struct icalproperty_enum_map enum_map[] = {\n";

  $idx = 10000;

  foreach $value (sort keys %valuemap) {
    
    next if !$value;    
    next if $value eq 'NO' or $prop eq 'ANY';

    my $ucv = join("",map {uc(lc($_));}  split(/-/,$value));    
    my @enums = @{$valuemap{$value}->{'enums'}};

    if(@enums){

      my ($c_autogen,$c_type) = @{$valuemap{$value}->{'C'}};
      
      unshift(@enums,"X");
      push(@enums,"NONE");

      foreach $e (@enums) {

	my $uce = join("",map {uc(lc($_));}  split(/-/,$e));
	
	if($e ne "X" and $e ne "NONE"){
	  $str = $e;
	} else {
	  $str = "";
	}

	print "    {ICAL_${ucv}_PROPERTY,ICAL_${ucv}_${uce},\"$str\" }, /*$idx*/\n";

	$idx++;
      }
      
    }
  }
  print "    {ICAL_NO_PROPERTY,0,\"\"}\n};\n\n";


}


if($opt_h){

  # Create the property enumerations list
  print "typedef enum icalproperty_kind {\n    ICAL_ANY_PROPERTY = 0,\n";
  foreach $prop (sort keys %propmap) {
    
    next if !$prop;
    
    next if $prop eq 'NO' or $prop eq 'ANY';
    
    my ($uc,$lc,$lcvalue,$ucvalue,$type) = fudge_data($prop);
    
    print "    ICAL_${uc}_PROPERTY, \n";
    
  }  
  print "    ICAL_NO_PROPERTY\n} icalproperty_kind;\n\n";


}


foreach $prop (sort keys %propmap) {

  next if !$prop;

  next if $prop eq 'NO' or $prop eq 'ANY';

  my ($uc,$lc,$lcvalue,$ucvalue,$type) = fudge_data($prop);

  
  my $pointer_check;
  if ($type =~ /\*/){
    $pointer_check = "icalerror_check_arg_rz( (v!=0),\"v\");\n" if $type =~ /\*/;
  } elsif ( $type eq "void" ){
    $pointer_check = "icalerror_check_arg_rv( (v!=0),\"v\");\n" if $type =~ /\*/;

  }    

  my $set_pointer_check = "icalerror_check_arg_rv( (v!=0),\"v\");\n" if $type =~ /\*/;

  if($opt_c) { # Generate C source

   if ($include_vanew) {
     print<<EOM;
icalproperty* icalproperty_vanew_${lc}($type v, ...){
   va_list args;
   struct icalproperty_impl *impl = icalproperty_new_impl(ICAL_${uc}_PROPERTY);   $pointer_check
   icalproperty_set_${lc}((icalproperty*)impl,v);
   va_start(args,v);
   icalproperty_add_parameters(impl, args);
   va_end(args);
   return (icalproperty*)impl;
}
EOM
}

 print<<EOM;
/* $prop */
icalproperty* icalproperty_new_${lc}($type v) {
   struct icalproperty_impl *impl = icalproperty_new_impl(ICAL_${uc}_PROPERTY);   $pointer_check
   icalproperty_set_${lc}((icalproperty*)impl,v);
   return (icalproperty*)impl;
}

void icalproperty_set_${lc}(icalproperty* prop, $type v){
    $set_pointer_check
    icalerror_check_arg_rv( (prop!=0),"prop");
    icalproperty_set_value(prop,icalvalue_new_${lcvalue}(v));
}
$type icalproperty_get_${lc}(icalproperty* prop){
    icalerror_check_arg( (prop!=0),"prop");
    return icalvalue_get_${lcvalue}(icalproperty_get_value(prop));
}
EOM


  } elsif ($opt_h) { # Generate C Header file

if ($include_vanew){
  print "icalproperty* icalproperty_vanew_${lc}($type v, ...);\n";
}

 print "\
/* $prop */\
icalproperty* icalproperty_new_${lc}($type v);\
void icalproperty_set_${lc}(icalproperty* prop, $type v);\
$type icalproperty_get_${lc}(icalproperty* prop);";
  
} 


} # This brace terminates the main loop



if ($opt_h){

print "\n\n#endif /*ICALPROPERTY_H*/\n"
}

