#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



package installer::windows::file;

use Digest::MD5; 
use installer::existence;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::worker;
use installer::windows::font;
use installer::windows::idtglobal;
use installer::windows::msiglobal;
use installer::windows::language;

##########################################################################
# Assigning one cabinet file to each file. This is requrired,
# if cabinet files shall be equivalent to packages.
##########################################################################

sub assign_cab_to_files
{
	my ( $filesref ) = @_;
	
	my $infoline = "";

	for ( my $i = 0; $i <= $#{$filesref}; $i++ )
	{
		if ( ! exists(${$filesref}[$i]->{'modules'}) ) { installer::exiter::exit_program("ERROR: No module assignment found for ${$filesref}[$i]->{'gid'} !", "assign_cab_to_files"); }
		my $module = ${$filesref}[$i]->{'modules'};
		# If modules contains a list of modules, only taking the first one.
		if ( $module =~ /^\s*(.*?)\,/ ) { $module = $1; }

		if ( ! exists($installer::globals::allcabinetassigns{$module}) ) { installer::exiter::exit_program("ERROR: No cabinet file assigned to module \"$module\" (${$filesref}[$i]->{'gid'}) !", "assign_cab_to_files"); }
		${$filesref}[$i]->{'assignedcabinetfile'} = $installer::globals::allcabinetassigns{$module};

		# Counting the files in each cabinet file
		if ( ! exists($installer::globals::cabfilecounter{${$filesref}[$i]->{'assignedcabinetfile'}}) )
		{
			$installer::globals::cabfilecounter{${$filesref}[$i]->{'assignedcabinetfile'}} = 1;
		}
		else
		{
			$installer::globals::cabfilecounter{${$filesref}[$i]->{'assignedcabinetfile'}}++;
		}
	}

	# logging the number of files in each cabinet file

	$installer::logger::Lang->print("\n");
	$installer::logger::Lang->print("Cabinet file content:\n");
	my $cabfile;
	foreach $cabfile ( sort keys %installer::globals::cabfilecounter )
	{
		$infoline = "$cabfile : $installer::globals::cabfilecounter{$cabfile} files\n";
		$installer::logger::Lang->print($infoline);
	}
	
	# assigning startsequencenumbers for each cab file
	
	my $offset = 1;
	foreach $cabfile ( sort keys %installer::globals::cabfilecounter )
	{
		my $filecount = $installer::globals::cabfilecounter{$cabfile};
		$installer::globals::cabfilecounter{$cabfile} = $offset;
		$offset = $offset + $filecount;
		
		$installer::globals::lastsequence{$cabfile} = $offset - 1;
	}
	
	# logging the start sequence numbers

	$installer::logger::Lang->print("\n");
	$installer::logger::Lang->print("Cabinet file start sequences:\n");
	foreach $cabfile ( sort keys %installer::globals::cabfilecounter )
	{
		$infoline = "$cabfile : $installer::globals::cabfilecounter{$cabfile}\n";
		$installer::logger::Lang->print($infoline);
	}

	# logging the last sequence numbers

	$installer::logger::Lang->print("\n");
	$installer::logger::Lang->print("Cabinet file last sequences:\n");
	foreach $cabfile ( sort keys %installer::globals::lastsequence )
	{
		$infoline = "$cabfile : $installer::globals::lastsequence{$cabfile}\n";
		$installer::logger::Lang->print($infoline);
	}
}

##########################################################################
# Assigning sequencenumbers to files. This is requrired,
# if cabinet files shall be equivalent to packages.
##########################################################################

sub assign_sequencenumbers_to_files
{
	my ( $filesref ) = @_;
	
	my %directaccess = ();
	my %allassigns = ();
	
	for ( my $i = 0; $i <= $#{$filesref}; $i++ )
	{
		my $onefile = ${$filesref}[$i];
		
		# Keeping order in cabinet files
		# -> collecting all files in one cabinet file
		# -> sorting files and assigning numbers

		# Saving counter $i for direct access into files array
		# "destination" of the file is a unique identifier ('Name' is not unique!)
		if ( exists($directaccess{$onefile->{'destination'}}) ) { installer::exiter::exit_program("ERROR: 'destination' at file not unique: $onefile->{'destination'}", "assign_sequencenumbers_to_files"); }
		$directaccess{$onefile->{'destination'}} = $i;

		my $cabfilename = $onefile->{'assignedcabinetfile'};
		# collecting files in cabinet files
		if ( ! exists($allassigns{$cabfilename}) )
		{
			my %onecabfile = ();
			$onecabfile{$onefile->{'destination'}} = 1;			
			$allassigns{$cabfilename} = \%onecabfile;
		}
		else
		{
			$allassigns{$cabfilename}->{$onefile->{'destination'}} = 1;
		}
	}
	
	# Sorting each hash and assigning numbers
	# The destination of the file determines the sort order, not the filename! 
	my $cabfile;
	foreach $cabfile ( sort keys %allassigns )
	{
		my $counter = $installer::globals::cabfilecounter{$cabfile};
		my $dest;
		foreach $dest ( sort keys %{$allassigns{$cabfile}} ) # <- sorting the destination!
		{
			my $directaccessnumber = $directaccess{$dest};
			${$filesref}[$directaccessnumber]->{'assignedsequencenumber'} = $counter;			
			$counter++;
		}
	}
}

#########################################################
# Create a shorter version of a long component name,
# because maximum length in msi database is 72.
# Attention: In multi msi installation sets, the short
# names have to be unique over all packages, because
# this string is used to create the globally unique id
# -> no resetting of
# %installer::globals::allshortcomponents
# after a package was created.
# Using no counter because of reproducibility.
#########################################################

sub generate_new_short_componentname
{
	my ($componentname) = @_;
	
	my $startversion = substr($componentname, 0, 60); # taking only the first 60 characters
	my $subid = installer::windows::msiglobal::calculate_id($componentname, 9); # taking only the first 9 digits
	my $shortcomponentname = $startversion . "_" . $subid;
	
	if ( exists($installer::globals::allshortcomponents{$shortcomponentname}) ) { installer::exiter::exit_program("Failed to create unique component name: \"$shortcomponentname\"", "generate_new_short_componentname"); }
	
	$installer::globals::allshortcomponents{$shortcomponentname} = 1;
	
	return $shortcomponentname;
}

###############################################
# Generating the component name from a file
###############################################

sub get_file_component_name
{
	my ($fileref, $filesref) = @_;

	my $componentname = "";
	
	# Special handling for files with ASSIGNCOMPOMENT
	
	my $styles = "";
	if ( $fileref->{'Styles'} ) { $styles = $fileref->{'Styles'}; }
	if ( $styles =~ /\bASSIGNCOMPOMENT\b/ )
	{
		$componentname = get_component_from_assigned_file($fileref->{'AssignComponent'}, $filesref);
	}
	else
	{
		# In this function exists the rule to create components from files
		# Rule:
		# Two files get the same componentid, if:
		# both have the same destination directory.
		# both have the same "gid" -> both were packed in the same zip file
		# All other files are included into different components!
	
		# my $componentname = $fileref->{'gid'} . "_" . $fileref->{'Dir'};

		# $fileref->{'Dir'} is not sufficient! All files in a zip file have the same $fileref->{'Dir'},
		# but can be in different subdirectories.
		# Solution: destination=share\Scripts\beanshell\Capitalise\capitalise.bsh
		# in which the filename (capitalise.bsh) has to be removed and all backslashes (slashes) are 
		# converted into underline.
	
		my $destination = $fileref->{'destination'};
		installer::pathanalyzer::get_path_from_fullqualifiedname(\$destination);
		$destination =~ s/\s//g;
		$destination =~ s/\\/\_/g;
		$destination =~ s/\//\_/g;
		$destination =~ s/\_\s*$//g;	# removing ending underline
	
		$componentname = $fileref->{'gid'} . "__" . $destination;

		# Files with different languages, need to be packed into different components.
		# Then the installation of the language specific component is determined by a language condition.

		if ( $fileref->{'ismultilingual'} )
		{
			my $officelanguage = $fileref->{'specificlanguage'};
			$componentname = $componentname . "_" . $officelanguage;	
		}

		$componentname = lc($componentname);	# componentnames always lowercase

		$componentname =~ s/\-/\_/g;			# converting "-" to "_"
		$componentname =~ s/\./\_/g;			# converting "-" to "_"

		# Attention: Maximum length for the componentname is 72
		# %installer::globals::allcomponents_in_this_database : resetted for each database	
		# %installer::globals::allcomponents : not resetted for each database
		# Component strings must be unique for the complete product, because they are used for
		# the creation of the globally unique identifier.	

		my $fullname = $componentname;  # This can be longer than 72
		
		if (( exists($installer::globals::allcomponents{$fullname}) ) && ( ! exists($installer::globals::allcomponents_in_this_database{$fullname}) ))
		{
			# This is not allowed: One component cannot be installed with different packages.
			installer::exiter::exit_program("ERROR: Component \"$fullname\" is already included into another package. This is not allowed.", "get_file_component_name");
		}
		
		if ( exists($installer::globals::allcomponents{$fullname}) )
		{
			$componentname = $installer::globals::allcomponents{$fullname};
		}
		else
		{
			if ( length($componentname) > 70 )
			{
				$componentname = generate_new_short_componentname($componentname); # This has to be unique for the complete product, not only one package
			}

			$installer::globals::allcomponents{$fullname} = $componentname;
			$installer::globals::allcomponents_in_this_database{$fullname} = 1;
		}

		# $componentname =~ s/gid_file_/g_f_/g;
		# $componentname =~ s/_extra_/_e_/g;
		# $componentname =~ s/_config_/_c_/g;
		# $componentname =~ s/_org_openoffice_/_o_o_/g;
		# $componentname =~ s/_program_/_p_/g;
		# $componentname =~ s/_typedetection_/_td_/g;
		# $componentname =~ s/_linguistic_/_l_/g;
		# $componentname =~ s/_module_/_m_/g;
		# $componentname =~ s/_optional_/_opt_/g;
		# $componentname =~ s/_packages/_pack/g;
		# $componentname =~ s/_menubar/_mb/g;
		# $componentname =~ s/_common_/_cm_/g;
		# $componentname =~ s/_export_/_exp_/g;
		# $componentname =~ s/_table_/_tb_/g;
		# $componentname =~ s/_sofficecfg_/_sc_/g;
		# $componentname =~ s/_soffice_cfg_/_sc_/g;
		# $componentname =~ s/_startmodulecommands_/_smc_/g;
		# $componentname =~ s/_drawimpresscommands_/_dic_/g;
		# $componentname =~ s/_basiccommands_/_bac_/g;
		# $componentname =~ s/_basicidecommands_/_baic_/g;
		# $componentname =~ s/_genericcommands_/_genc_/g;
		# $componentname =~ s/_bibliographycommands_/_bibc_/g;
		# $componentname =~ s/_gentiumbookbasicbolditalic_/_gbbbi_/g;
		# $componentname =~ s/_share_/_s_/g;
		# $componentname =~ s/_extension_/_ext_/g;
		# $componentname =~ s/_extensions_/_exs_/g;
		# $componentname =~ s/_modules_/_ms_/g;
		# $componentname =~ s/_uiconfig_zip_/_ucz_/g;
		# $componentname =~ s/_productivity_/_pr_/g;
		# $componentname =~ s/_wizard_/_wz_/g;
		# $componentname =~ s/_import_/_im_/g;
		# $componentname =~ s/_javascript_/_js_/g;
		# $componentname =~ s/_template_/_tpl_/g;
		# $componentname =~ s/_tplwizletter_/_twl_/g;
		# $componentname =~ s/_beanshell_/_bs_/g;
		# $componentname =~ s/_presentation_/_bs_/g;
		# $componentname =~ s/_columns_/_cls_/g;
		# $componentname =~ s/_python_/_py_/g;

		# $componentname =~ s/_tools/_ts/g;
		# $componentname =~ s/_transitions/_trs/g;
		# $componentname =~ s/_scriptbinding/_scrb/g;
		# $componentname =~ s/_spreadsheet/_ssh/g;
		# $componentname =~ s/_publisher/_pub/g;
		# $componentname =~ s/_presenter/_pre/g;
		# $componentname =~ s/_registry/_reg/g;

		# $componentname =~ s/screen/sc/g;
		# $componentname =~ s/wordml/wm/g;
		# $componentname =~ s/openoffice/oo/g;
	}

	return $componentname;	
}

####################################################################
# Returning the component name for a defined file gid.
# This is necessary for files with flag ASSIGNCOMPOMENT
####################################################################

sub get_component_from_assigned_file
{
	my ($gid, $filesref) = @_;

	my $onefile = installer::existence::get_specified_file($filesref, $gid);
	my $componentname = "";
	if ( $onefile->{'componentname'} ) { $componentname = $onefile->{'componentname'}; }
	else { installer::exiter::exit_program("ERROR: No component defined for file: $gid", "get_component_from_assigned_file"); }
	
	return $componentname;
}

####################################################################
# Generating the special filename for the database file File.idt
# Sample: CONTEXTS, CONTEXTS1
# This name has to be unique.
# In most cases this is simply the filename.
####################################################################

sub generate_unique_filename_for_filetable ($$)
{
	my ($fileref, $component) = @_;

	# This new filename has to be saved into $fileref, because this is needed to find the source.
	# The filename sbasic.idx/OFFSETS is changed to OFFSETS, but OFFSETS is not unique.
	# In this procedure names like OFFSETS5 are produced. And exactly this string has to be added to
	# the array of all files.

	my $uniquefilename = "";
	my $counter = 0;

	if ( $fileref->{'Name'} ) { $uniquefilename = $fileref->{'Name'}; }

	installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$uniquefilename);	# making /registry/schema/org/openoffice/VCL.xcs to VCL.xcs 

	# Reading unique filename with help of "Component_" in File table from old database	
	if (( $installer::globals::prepare_winpatch ) && ( exists($installer::globals::savedmapping{"$component/$uniquefilename"}) ))
	{
		# If we have a FTK mapping for this component/file, use it.
		$installer::globals::savedmapping{"$component/$uniquefilename"} =~ m/^(.*);/;
		$uniquefilename = $1;
 		$lcuniquefilename = lc($uniquefilename);
		$installer::globals::alluniquefilenames{$uniquefilename} = 1;
		$installer::globals::alllcuniquefilenames{$lcuniquefilename} = 1;
		return $uniquefilename;
	}

	$uniquefilename =~ s/\-/\_/g;		# no "-" allowed
	$uniquefilename =~ s/\@/\_/g;		# no "@" allowed
	$uniquefilename =~ s/\$/\_/g;		# no "$" allowed
	$uniquefilename =~ s/^\s*\./\_/g;		# no "." at the beginning allowed allowed
	$uniquefilename =~ s/^\s*\d/\_d/g;		# no number at the beginning allowed allowed (even file "0.gif", replacing to "_d.gif")
	$uniquefilename =~ s/org_openoffice_/ooo_/g;	# shorten the unique file name

	my $lcuniquefilename = lc($uniquefilename);	# only lowercase names

	my $newname = 0;

	if ( ! exists($installer::globals::alllcuniquefilenames{$lcuniquefilename}) &&
	     ! exists($installer::globals::savedrevmapping{$lcuniquefilename}) )
	{
		$installer::globals::alluniquefilenames{$uniquefilename} = 1;
		$installer::globals::alllcuniquefilenames{$lcuniquefilename} = 1;
		$newname = 1;
	}

	if ( ! $newname )
	{
		# adding a number until the name is really unique: OFFSETS, OFFSETS1, OFFSETS2, ...
		# But attention: Making "abc.xcu" to "abc1.xcu"
		
		my $uniquefilenamebase = $uniquefilename;
		
		do
		{
			$counter++;

			if ( $uniquefilenamebase =~ /\./ )
			{
				$uniquefilename = $uniquefilenamebase;
				$uniquefilename =~ s/\./$counter\./;
			}
			else
			{
				$uniquefilename = $uniquefilenamebase . $counter;
			}

			$newname = 0;
			$lcuniquefilename = lc($uniquefilename);	# only lowercase names

			if ( ! exists($installer::globals::alllcuniquefilenames{$lcuniquefilename}) &&
			     ! exists($installer::globals::savedrevmapping{$lcuniquefilename}) )
			{
				$installer::globals::alluniquefilenames{$uniquefilename} = 1;
				$installer::globals::alllcuniquefilenames{$lcuniquefilename} = 1;
				$newname = 1;
			}
		}
		until ( $newname ) 
	}

	return $uniquefilename;	
}

####################################################################
# Generating the special file column for the database file File.idt
# Sample: NAMETR~1.TAB|.nametranslation.table
# The first part has to be 8.3 conform.
####################################################################

sub generate_filename_for_filetable ($$)
{
	my ($fileref, $shortnamesref) = @_;

	my $returnstring = "";

	my $filename = $fileref->{'Name'};	
	
	installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$filename);	# making /registry/schema/org/openoffice/VCL.xcs to VCL.xcs 
	
	my $shortstring;
	
	# Reading short string with help of "FileName" in File table from old database	
	if (( $installer::globals::prepare_winpatch ) && ( exists($installer::globals::savedmapping{"$fileref->{'componentname'}/$filename"}) ))
	{
		$installer::globals::savedmapping{"$fileref->{'componentname'}/$filename"} =~ m/.*;(.*)/;
		if ($1 ne '')
		{
			$shortstring = $1;
		}
		else
		{
			$shortstring = installer::windows::idtglobal::make_eight_three_conform_with_hash($filename, "file", $shortnamesref);
		}
	}
	else
	{
		$shortstring = installer::windows::idtglobal::make_eight_three_conform_with_hash($filename, "file", $shortnamesref);
	}
	
	if ( $shortstring eq $filename ) { $returnstring = $filename; }	# nothing changed
	else {$returnstring = $shortstring . "\|" . $filename; }
	
	return $returnstring;
}

#########################################
# Returning the filesize of a file
#########################################

sub get_filesize
{
	my ($fileref) = @_;

	my $file = $fileref->{'sourcepath'};

	my $filesize;
	
	if ( -f $file )	# test of existence. For instance services.rdb does not always exist
	{
		$filesize = ( -s $file );	# file size can be "0"
	}
	else
	{
		$filesize = -1;
	}
		
	return $filesize;
}

#############################################
# Returning the file version, if required
# Sample: "8.0.1.8976";
#############################################

sub get_fileversion
{
	my ($onefile, $allvariables, $styles) = @_;

	my $fileversion = "";

	if ( $allvariables->{'USE_FILEVERSION'} )
	{
		if ( ! $allvariables->{'LIBRARYVERSION'} ) { installer::exiter::exit_program("ERROR: USE_FILEVERSION is set, but not LIBRARYVERSION", "get_fileversion"); } 
		my $libraryversion = $allvariables->{'LIBRARYVERSION'};
		if ( $libraryversion =~ /^\s*(\d+)\.(\d+)\.(\d+)\s*$/ )
		{
			my $major = $1;
			my $minor = $2;
			my $micro = $3;
			my $concat = 100 * $minor + $micro;
			$libraryversion = $major . "\." . $concat;
		}
		my $vendornumber = 0;
		if ( $allvariables->{'VENDORPATCHVERSION'} ) { $vendornumber = $allvariables->{'VENDORPATCHVERSION'}; }
		$fileversion = $libraryversion . "\." . $installer::globals::buildid . "\." . $vendornumber;
		if ( $onefile->{'FileVersion'} ) { $fileversion = $onefile->{'FileVersion'}; } # overriding FileVersion in scp

		# if ( $styles =~ /\bFONT\b/ )
		# {
		#	my $newfileversion = installer::windows::font::get_font_version($onefile->{'sourcepath'});
		#	if ( $newfileversion != 0 ) { $fileversion = $newfileversion; }
		# }
	}
	
	if ( $installer::globals::prepare_winpatch ) { $fileversion = ""; } # Windows patches do not allow this version # -> who says so?
		
	return $fileversion;
}

#############################################
# Returning the Windows language of a file
#############################################

sub get_language_for_file
{
	my ($fileref) = @_;
	
	my $language = "";
	
	if ( $fileref->{'specificlanguage'} ) { $language = $fileref->{'specificlanguage'}; }
	
	if ( $language eq "" )
	{
		$language = 0;  # language independent 
		# If this is not a font, the return value should be "0" (Check ICE 60)
		my $styles = "";
		if ( $fileref->{'Styles'} ) { $styles = $fileref->{'Styles'}; }
		if ( $styles =~ /\bFONT\b/ ) { $language = ""; }
	}
	else
	{
		$language = installer::windows::language::get_windows_language($language);
	}
	
	return $language;	
}

####################################################################
# Creating a new KeyPath for components in TemplatesFolder.
####################################################################

sub generate_registry_keypath
{
	my ($onefile) = @_;

	my $keypath = $onefile->{'Name'};
	$keypath =~ s/\.//g;
	$keypath = lc($keypath);
	$keypath = "userreg_" . $keypath;

	return $keypath;	
}

###################################################################
# Collecting further conditions for the component table.
# This is used by multilayer products, to enable installation 
# of separate layers.
###################################################################

sub get_tree_condition_for_component
{
	my ($onefile, $componentname) = @_;
	
	if ( $onefile->{'destination'} )
	{
		my $dest = $onefile->{'destination'};
		
		# Comparing the destination path with
		# $installer::globals::hostnametreestyles{$hostname} = $treestyle;
		# (-> hostname is the key, the style the value!)

		foreach my $hostname ( keys %installer::globals::hostnametreestyles )
		{
			if (( $dest eq $hostname ) || ( $dest =~ /^\s*\Q$hostname\E\\/ ))
			{
				# the value is the style
				my $style = $installer::globals::hostnametreestyles{$hostname};
				# the condition is saved in %installer::globals::treestyles
				my $condition = $installer::globals::treestyles{$style};
				# Saving condition to be added in table Property
				$installer::globals::usedtreeconditions{$condition} = 1;
				$condition = $condition . "=1";
				# saving this condition
				$installer::globals::treeconditions{$componentname} = $condition;
			
				# saving also at the file, for usage in fileinfo
				$onefile->{'layer'} = $installer::globals::treelayername{$style};
			}
		}
	}
}

############################################
# Collecting all short names, that are
# already used by the old database
############################################

sub collect_shortnames_from_old_database
{
	my ($uniquefilenamehashref, $shortnameshashref) = @_;

	foreach my $key ( keys %{$uniquefilenamehashref} )
	{
		my $value = $uniquefilenamehashref->{$key};  # syntax of $value: ($uniquename;$shortname)

		if ( $value =~ /^\s*(.*?)\;\s*(.*?)\s*$/ )
		{
			my $shortstring = $2;
			$shortnameshashref->{$shortstring} = 1;	# adding the shortname to the array of all shortnames
		}
	}
}

############################################
# Creating the file File.idt dynamically
############################################

sub create_files_table ($$$$)
{
	my ($filesref, $allfilecomponentsref, $basedir, $allvariables) = @_;

	$installer::logger::Lang->add_timestamp("Performance Info: File Table start");

	# Structure of the files table:
	# File Component_ FileName FileSize Version Language Attributes Sequence
	# In this function, all components are created.
	#
	# $allfilecomponentsref is empty at the beginning

	my $infoline;

	my @allfiles = ();
	my @filetable = ();
	my @filehashtable = ();
	my %allfilecomponents = ();
	my $counter = 0;

	if ( $^O =~ /cygwin/i ) { installer::worker::generate_cygwin_pathes($filesref); }
	
	# The filenames must be collected because of uniqueness
	# 01-44-~1.DAT, 01-44-~2.DAT, ...
	# my @shortnames = ();
	my %shortnames = ();
	
	installer::windows::idtglobal::write_idt_header(\@filetable, "file");
	installer::windows::idtglobal::write_idt_header(\@filehashtable, "filehash");
	
	for ( my $i = 0; $i <= $#{$filesref}; $i++ )
	{
		my %file = ();

		my $onefile = ${$filesref}[$i];	

		my $styles = "";
		if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }
		if (( $styles =~ /\bJAVAFILE\b/ ) && ( ! ($allvariables->{'JAVAPRODUCT'} ))) { next; }

		$file{'Component_'} = get_file_component_name($onefile, $filesref);
		$file{'File'} = generate_unique_filename_for_filetable($onefile, $file{'Component_'});
	
		$onefile->{'uniquename'} = $file{'File'};
		$onefile->{'componentname'} = $file{'Component_'};

		# Collecting all components
		# if (!(installer::existence::exists_in_array($file{'Component_'}, $allfilecomponentsref))) { push(@{$allfilecomponentsref}, $file{'Component_'}); }

		if ( ! exists($allfilecomponents{$file{'Component_'}}) ) { $allfilecomponents{$file{'Component_'}} = 1; }

		$file{'FileName'} = generate_filename_for_filetable($onefile, \%shortnames);

		$file{'FileSize'} = get_filesize($onefile);

		$file{'Version'} = get_fileversion($onefile, $allvariables, $styles);

		$file{'Language'} = get_language_for_file($onefile);
		
		if ( $styles =~ /\bDONT_PACK\b/ ) { $file{'Attributes'} = "8192"; }
		else { $file{'Attributes'} = "16384"; }

		# $file{'Attributes'} = "16384"; 	# Sourcefile is packed
		# $file{'Attributes'} = "8192"; 	# Sourcefile is unpacked

		$installer::globals::insert_file_at_end = 0;
		$counter++;
		$file{'Sequence'} = $counter;

		$onefile->{'sequencenumber'} = $file{'Sequence'};

		my $oneline = $file{'File'} . "\t" . $file{'Component_'} . "\t" . $file{'FileName'} . "\t" 
				. $file{'FileSize'} . "\t" . $file{'Version'} . "\t" . $file{'Language'} . "\t"
				. $file{'Attributes'} . "\t" . $file{'Sequence'} . "\n";
	
		push(@filetable, $oneline);

		if ( ! $installer::globals::insert_file_at_end ) { push(@allfiles, $onefile); }

		# Collecting all component conditions
		if ( $onefile->{'ComponentCondition'} )
		{			
			if ( ! exists($installer::globals::componentcondition{$file{'Component_'}}))
			{
				$installer::globals::componentcondition{$file{'Component_'}} = $onefile->{'ComponentCondition'};
			}
		}
		
		# Collecting also all tree conditions for multilayer products
		get_tree_condition_for_component($onefile, $file{'Component_'});

		# Collecting all component names, that have flag VERSION_INDEPENDENT_COMP_ID
		# This should be all components with constant API, for example URE
		if ( $styles =~ /\bVERSION_INDEPENDENT_COMP_ID\b/ )
		{
			$installer::globals::base_independent_components{$onefile->{'componentname'}} = 1;
		}

		# Collecting all component ids, that are defined at files in scp project (should not be used anymore)
		if ( $onefile->{'CompID'} )
		{			
			if ( ! exists($installer::globals::componentid{$onefile->{'componentname'}}))
			{
				$installer::globals::componentid{$onefile->{'componentname'}} = $onefile->{'CompID'};
			}
			else
			{
				if ( $installer::globals::componentid{$onefile->{'componentname'}} ne $onefile->{'CompID'} )
				{
					installer::exiter::exit_program("ERROR: There is already a ComponentID for component \"$onefile->{'componentname'}\" : \"$installer::globals::componentid{$onefile->{'componentname'}}\" . File \"$onefile->{'gid'}\" uses \"$onefile->{'CompID'}\" !", "create_files_table");
				}
			}

			# Also checking vice versa. Is this ComponentID already used? If yes, is the componentname the same?
	
			if ( ! exists($installer::globals::comparecomponentname{$onefile->{'CompID'}}))
			{
				$installer::globals::comparecomponentname{$onefile->{'CompID'}} = $onefile->{'componentname'};
			}
			else
			{
				if ( $installer::globals::comparecomponentname{$onefile->{'CompID'}} ne $onefile->{'componentname'} )
				{
					installer::exiter::exit_program("ERROR: There is already a component for ComponentID \"$onefile->{'CompID'}\" : \"$installer::globals::comparecomponentname{$onefile->{'CompID'}}\" . File \"$onefile->{'gid'}\" has same component id but is included in component \"$onefile->{'componentname'}\" !", "create_files_table");
				}				
			}
		}

		# Collecting all language specific conditions
		# if ( $onefile->{'haslanguagemodule'} )
		if ( $onefile->{'ismultilingual'} )
		{
			if ( $onefile->{'ComponentCondition'} ) { installer::exiter::exit_program("ERROR: Cannot set language condition. There is already another component condition for file $onefile->{'gid'}: \"$onefile->{'ComponentCondition'}\" !", "create_files_table"); }

			if ( $onefile->{'specificlanguage'} eq "" ) { installer::exiter::exit_program("ERROR: There is no specific language for file at language module: $onefile->{'gid'} !", "create_files_table"); }
			my $locallanguage = $onefile->{'specificlanguage'};
			my $property = "IS" . $file{'Language'};
			my $value = 1;
			my $condition = $property . "=" . $value;
			
			$onefile->{'ComponentCondition'} = $condition;

			if ( exists($installer::globals::componentcondition{$file{'Component_'}}))
			{
				if ( $installer::globals::componentcondition{$file{'Component_'}} ne $condition ) { installer::exiter::exit_program("ERROR: There is already another component condition for file $onefile->{'gid'}: \"$installer::globals::componentcondition{$file{'Component_'}}\" and \"$condition\" !", "create_files_table"); }
			}
			else
			{
				$installer::globals::componentcondition{$file{'Component_'}} = $condition;
			}		

			# collecting all properties for table Property
			if ( ! exists($installer::globals::languageproperties{$property}) ) { $installer::globals::languageproperties{$property} = $value; }
		}

		if ( $installer::globals::prepare_winpatch )
		{
			my $path = $onefile->{'sourcepath'};
			if ( $^O =~ /cygwin/i ) { $path = $onefile->{'cyg_sourcepath'}; }

			open(FILE, $path) or die "ERROR: Can't open $path for creating file hash";
			binmode(FILE);
			my $hashinfo = pack("l", 20);
			$hashinfo .= Digest::MD5->new->addfile(*FILE)->digest;

			my @i = unpack ('x[l]l4', $hashinfo);
			$oneline = $file{'File'} . "\t" .
				"0" . "\t" .
				$i[0] . "\t" .
				$i[1] . "\t" .
				$i[2] . "\t" .
				$i[3] . "\n";
			push (@filehashtable, $oneline);
		}

		# Saving the sequence number in a hash with uniquefilename as key.
		# This is used for better performance in "save_packorder"
		$installer::globals::uniquefilenamesequence{$onefile->{'uniquename'}} = $onefile->{'sequencenumber'};
		
		# Special handling for files in PREDEFINED_OSSHELLNEWDIR. These components
		# need as KeyPath a RegistryItem in HKCU
		my $destdir = "";
		if ( $onefile->{'Dir'} ) { $destdir = $onefile->{'Dir'}; }

		if (( $destdir =~ /\bPREDEFINED_OSSHELLNEWDIR\b/ ) || ( $onefile->{'needs_user_registry_key'} ))
		{
			my $keypath = generate_registry_keypath($onefile);
			$onefile->{'userregkeypath'} = $keypath;
			push(@installer::globals::userregistrycollector, $onefile);
			$installer::globals::addeduserregitrykeys = 1;
		}	
	}

	# putting content from %allfilecomponents to $allfilecomponentsref for later usage
	foreach $localkey (keys %allfilecomponents ) { push( @{$allfilecomponentsref}, $localkey); }

	my $filetablename = $basedir . $installer::globals::separator . "File.idt";
	installer::files::save_file($filetablename ,\@filetable);
	$installer::logger::Lang->print("\n");
	$installer::logger::Lang->printf("Created idt file: %s\n", $filetablename); 

	$installer::logger::Lang->add_timestamp("Performance Info: File Table end");
	
	my $filehashtablename = $basedir . $installer::globals::separator . "MsiFileHash.idt";
	installer::files::save_file($filehashtablename ,\@filehashtable);
	$installer::logger::Lang->print("\n");
	$installer::logger::Lang->printf("Created idt file: %s\n", $filehashtablename);

	# Now the new files can be added to the files collector (only in update packaging processes)
	if ( $installer::globals::newfilesexist )
	{
		foreach my $seq (sort keys %installer::globals::newfilescollector) { push(@allfiles, $installer::globals::newfilescollector{$seq}) }
	}

	return \@allfiles;
}

1;
