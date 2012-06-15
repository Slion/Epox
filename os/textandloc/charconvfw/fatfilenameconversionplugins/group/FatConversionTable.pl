#
# Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description: 
# This tool takes a valid .txt file as an input parameter and converts it into a .cpp file
#

if (@ARGV != 1 && @ARGV != 2)
	{
	print <<EOD;
Usage:explanation.....
EOD
	exit(1);
	}

# Removes the extenstion from the filename
$ARGV[0] =~ m/(.*)\..*/;
my $root = $1;
$root =~ m/.*[\\\/]([^\\\/]*)$/;
my $header_to_include = $1;

if (@ARGV == 2)
	{
	$ARGV[1] =~ m/(.*)\..*/;
	$root = $1;
	}

open (IN, "<$ARGV[0]") or die ("Error: $ARGV[0] $!");

open (CPP, ">$root.cpp") or die ("Error: $ARGV[0] Can't open cpp file");

print CPP <<EOD;
//
// Auto-generated by the FatConversiontable tool - Do not edit!!!
//

#include <e32std.h>
#include <e32def.h>
#include <e32des8.h> 
#include "convdatastruct.h"

EOD
print CPP "const TLeadOrSingle TConvDataStruct\:\:KFirstByteConversions\[\]=\n";
print CPP "\t\t\{\n";

my $input;
my $count = 0;
my $count2 = 0;
my $tailIncrement = hex(40);
my $index = 0;
my $test = 0;
my $leadByte = hex(00);

until (eof IN) {	#until end of file

#ignore lines that start with a '#' or which are in the range of 0x00 - 0x7F
do {
	$input = readline(IN) or die ("Error: Can't read line");
}while ($input =~ m/^$*#/ || $input=~ m/^0x[0-7][0-9A-F]\t/);

#single bytes get stored into 'SingleByte' array
if($input =~ m/^0x(..)\t(.*)\t(#.*)$/) 
	{
	my $SingleByte = hex($1);
	my $Unicode = $2;
	my $Note = $3;
	
	if ($Unicode=~ m/0x..../)
		{
		@SingleByte[$count] = (["$SingleByte", "$Unicode"]);
		$count = $count + 1;
		}
	if($Note eq '#DBCS LEAD BYTE')
		{
		@SingleByte[$count] = (["$SingleByte", "0"]);
		$count = $count + 1;
		}
	if ($Note eq '#UNDEFINED')
		{
		@SingleByte[$count] = (["$SingleByte", "0xFFFD"]);		
		$count = $count + 1;
		}
	}
	
#double bytes get stored into 'DoubleByte' array, whereby the tail bytes 
#must not have any gaps from 0x40 to 0xFF;
elsif($input=~ m/^0x(..)(..)\t(0x....)\t#.*/)
	{
	my $ForeignLead = hex($1);
	my $ForeignTail = hex($2);
	my $Unicode = $3;
	$test = 1;		
	
	if($leadByte==0)
		{
		$leadByte=$ForeignLead;
		}
		
	if($leadByte!=$ForeignLead)
		{
		if($tailIncrement!=hex(40))
			{
			while($tailIncrement<=hex(FF))
				{
				@DoubleByte[$count2] = (["$leadByte", "0xFFFD","$tailIncrement"]);
				$count2++;	
				$tailIncrement++;
				}
			}
		$tailIncrement=hex(40);
		$leadByte=$ForeignLead;
		}
		
	while($tailIncrement<=$ForeignTail)
		{
		if($tailIncrement==$ForeignTail)
			{
			@DoubleByte[$count2] = (["$ForeignLead", "$Unicode","$ForeignTail"]);		
			}
		else
			{
			@DoubleByte[$count2] = (["$leadByte", "0xFFFD","$tailIncrement"]);
			}
		$count2++;	
		if($tailIncrement==hex(FF))
			{
			$tailIncrement=hex(40);
			$leadByte++;
			}
		else
			{
			$tailIncrement++;
			}
		}
	}
} #end of loop

#my $d=0;
#for ($d=0;$d<$count2;$d++)
#{
#print CPP"$DoubleByte[$d][0], $DoubleByte[$d][1], $DoubleByte[$d][2]\t\n";
#}

#checks if tail byte ended uncompleted (i.e. ends with xFD) and completes it to xFF;
if($test==1)
	{
	my $counter = $count2-1;
	$test=0;
	
	if($DoubleByte[$counter][2]<0xFF)
		{
		my $temp = $DoubleByte[$counter][0];
		my $temp2 = $DoubleByte[$counter][2];
		do
			{
			$temp2++;
			@DoubleByte[$count2] = (["$temp", "0xFFFD","$temp2"]);
			$count2 = $count2 + 1;
			}while($temp2<0xFF);
		}
	}

my $position = 0;
my $position2 = 0;
my $x=0;
my $y=0;

#get the positions of single/lead bytes
for($x=0; $x<$count; $x++)
	{
	my $found=0;
	if($SingleByte[$x][1] eq '0') #if lead-byte...
		{	
		for($y=0; $y<$count2; $y++)
			{
			if($SingleByte[$x][0] == $DoubleByte[$y][0])
				{
				$position = $y;
				$position2 = $y + 192;
				$found = 1;
				$y=$count2;
				}	
			}		
		if($found==1)
			{
			print CPP "\t\t\{$SingleByte[$x][1], $position\},\n";
			}
		else
			{
			print CPP "\t\t\{0xFFFD, $position2\},\n";
			}
		}
	else
		{
		print CPP "\t\t\{$SingleByte[$x][1], $position2\},\n";
		}
	}
print CPP "\t\t\};\n\n";

#print double bytes
print CPP "const TUint16 TConvDataStruct\:\:KDoubleByteConversions\[\]=\n";
print CPP "\t\t\{";
my $newLine = 0;
if($count2>0)
	{
	for ($i=0; $i<$count2; $i++)
		{
		if($newLine==15)	#use a newline every 15 entries, to maintain user-friendliness
			{
			print CPP "\n\t\t";
			$newLine = 0;
			}
		print CPP "$DoubleByte[$i][1],";
		$newLine++;
		}
	}
else
	{
	print CPP "0x00";
	}
print CPP "\};\n\n";
if($count2>0)
	{
	$count2=$count2-1;
	}
print CPP "const TUint16 TConvDataStruct\:\:KDoubleByteConversionLength = $count2;\n\n";
print CPP "const TUint8 TConvDataStruct\:\:KMinTrailByte = 0x40;\n\n";
print CPP "const TUint8 TConvDataStruct\:\:KMaxTrailByte = 0xFF;\n\n";

print CPP <<EOD;
TInt TConvDataStruct::ConvertSingleUnicode(TInt aUnicode, TInt& aTrailByte)
	{
	aTrailByte = KErrNotFound;
	
	//single byte conversion check
	for(TInt i=0;i!=0x80;++i)
		{
		if(KFirstByteConversions[i].iUnicodeIfSingle==aUnicode)
			return i+0x80;
		}
	//double byte conversion check
	for(TInt j=0;j<=KDoubleByteConversionLength;++j)
		{
		if(KDoubleByteConversions[j] == aUnicode)
			{
			for(TInt k=0x7F;k>=0;--k)
				{
				TInt temp = j-KFirstByteConversions[k].iDoubleByteIndex;
				if(0<=temp)
					{
					aTrailByte = KMinTrailByte + temp;
					return k+0x80;
					}
				}
			}
		}
	return KErrNotFound;
EOD
print CPP "\t\}\n";
