#!/usr/bin/perl

=head1 NAME

    download_external_libraries.pl - Load missing tarballs specified in main/external_libs.lst.

=head1 SYNOPSIS

    For downloading external libraries (typically from the main/bootstrap script):

    download_external_libraries(<data-file-name>);

=head1 DESCRIPTION

    The contents of the main/external_libs.lst file are used to determine the
    external library tarballs that are missing from ext_sources/.

    Individual libraries can be ignored depending on the values of environment variables.

    Format of the main/external_libs.lst file:

    The file is line based.
    Comments start with a # and go to the end of the line and are ignored.
    Lines that are empty or contain only spaces and/or comments are ignored.

    All other lines can have one of two forms:
    - A variable definition of the form <name>=<value>.
    - A conditional block start in the form "if (<expression>)"

    Variables defined in a conditional block are only visible in this block and
    replace the definition of global variables and variables earlier in the same
    block.
    Some variables have special names:
    - MD5 is the expected MD5 sum of the library tarball.
    - URL1 to URL9 specify from where to download the tarball.  The urls are tried in order.
      The first successful download (download completed and MD5 sum match) stops the iteration.

    Expressions are explained below in the comment of EvaluateExpression().

    A library is only regarded if its conditional expression evaluates to 1.

    Example:

    DefaultSite=http://some-internet-site.org
    if ( true )
        MD5 = 0123456789abcdef0123456789abcdef
        name = library-1.0.tar.gz
        URL1 = http://some-other-internet-site.org/another-name.tgz
        URL2 = $(DefaultSite)$(MD5)-$(name)

    This tries to load a library first from some-other-internet-site.org and if
    that fails from some-internet-site.org.  The library is stored as $(MD5)-$(name)
    even when it is loaded as another-name.tgz.

=cut


use strict;

use File::Spec;
use File::Path;
use File::Basename;
use LWP::UserAgent;
use Digest::MD5;
use URI;

my $Debug = 1;

my $LocalEnvironment = undef;
my $GlobalEnvironment = {};
my @Missing = ();




=head3 ProcessDataFile

    Read the data file, typically named main/external_libs.lst, find the external
    library tarballs that are not yet present in ext_sources/ and download them.

=cut
sub ProcessDataFile ($)
{
    my $filename = shift;

    my $destination = $ENV{'TARFILE_LOCATION'};

    die "can not open data file $filename" if ! -e $filename;

    my $current_selector_value = 1;
    my @URLHeads = ();
    my @download_requests = ();

    open my $in, $filename;
    while (my $line = <$in>)
    {
        # Remove leading and trailing space and comments
        $line =~ s/^\s+//;
        $line =~ s/\s+$//;
        $line =~ s/\s*#.*$//;

        # Ignore empty lines.
        next if $line eq "";

        # An "if" statement starts a new block.
        if ($line =~ /^\s*if\s*\(\s*(.*?)\s*\)\s*$/)
        {
            ProcessLastBlock();

            $LocalEnvironment = { 'selector' => $1 };
        }

        # Lines of the form name = value define a local variable.
        elsif ($line =~ /^\s*(\S+)\s*=\s*(.*?)\s*$/)
        {
            if (defined $LocalEnvironment)
            {
                $LocalEnvironment->{$1} = $2;
            }
            else
            {
                $GlobalEnvironment->{$1} = $2;
            }
        }
        else
        {
            die "can not parse line $line\n";
        }
    }

    ProcessLastBlock();

    Download(\@download_requests, \@URLHeads);
}




=head3 ProcessLastBlock

    Process the last definition of an external library.
    If there is not last block, true for the first "if" statement, then the call is ignored.

=cut
sub ProcessLastBlock ()
{
    # Return if no block is defined.
    return if ! defined $LocalEnvironment;

    # Ignore the block if the selector does not match.
    if ( ! EvaluateExpression(SubstituteVariables($LocalEnvironment->{'selector'})))
    {
        printf("ignoring %s because its prerequisites are not fulfilled\n", GetValue('name'));
    }
    else
    {
        my $name = GetValue('name');

        if ( ! IsPresent($name, GetValue('MD5')))
        {
            AddDownloadRequest($name);
        }
    }
}




=head3 AddDownloadRequest($name)

    Add a request for downloading the library $name to @Missing.
    Collect all available URL[1-9] variables as source URLs.

=cut
sub AddDownloadRequest ($)
{
    my $name = shift;

    print "adding download request for $name\n";

    my $urls = [];
    my $url = GetValue('URL');
    push @$urls, SubstituteVariables($url) if (defined $url);
    for (my $i=1; $i<10; ++$i)
    {
        $url = GetValue('URL'.$i);
        next if ! defined $url;
        push @$urls, SubstituteVariables($url);
    }

    push @Missing, [$name, GetValue('MD5'), $urls];
}




=head3 GetValue($variable_name)

    Return the value of the variable with name $variable_name from the local
    environment or, if not defined there, the global environment.

=cut
sub GetValue ($)
{
    my $variable_name = shift;

    my $candidate = $LocalEnvironment->{$variable_name};
    return $candidate if defined $candidate;

    return $GlobalEnvironment->{$variable_name};
}



=head3 SubstituteVariables($text)

    Replace all references to variables in $text with the respective variable values.
    This is done repeatedly until no variable reference remains.

=cut
sub SubstituteVariables ($)
{
    my $text = shift;

    my $infinite_recursion_guard = 100;
    while ($text =~ /^(.*?)\$\(([^)]+)\)(.*)$/)
    {
        my ($head,$name,$tail) = ($1,$2,$3);
        my $value = GetValue($name);
        die "can evaluate variable $name" if ! defined $value;
        $text = $head.$value.$tail;

        die "(probably) detected an infinite recursion in variable definitions" if --$infinite_recursion_guard<=0;
    }

    return $text;
}




=head3 EvaluateExpression($expression)

    Evaluate the $expression of an "if" statement to either 0 or 1.  It can
    be a single term (see EvaluateTerm for a description), or several terms
    separated by either all ||s or &&s.  A term can also be an expression
    enclosed in parantheses.

=cut
sub EvaluateExpression ($)
{
    my $expression = shift;

    # Evaluate sub expressions enclosed in parantheses.
    while ($expression =~ /^(.*)\(([^\(\)]+)\)(.*)$/)
    {
        $expression = $1 . (EvaluateExpression($2) ? " true " : " false ") . $3;
    }

    if ($expression =~ /&&/ && $expression =~ /\|\|/)
    {
        die "expression can contain either && or || but not both at the same time";
    }
    elsif ($expression =~ /&&/)
    {
        foreach my $term (split (/\s*&&\s*/,$expression))
        {
            return 0 if ! EvaluateTerm($term);
        }
        return 1;
    }
    elsif ($expression =~ /\|\|/)
    {
        foreach my $term (split (/\s*\|\|\s*/,$expression))
        {
            return 1 if EvaluateTerm($term);
        }
        return 0;
    }
    else
    {
        return EvaluateTerm($expression);
    }
}




=head3 EvaluateTerm($term)

    Evaluate the $term to either 0 or 1.
    A term is either the literal "true", which evaluates to 1, or an expression
    of the form NAME=VALUE or NAME!=VALUE.  NAME is the name of an environment
    variable and VALUE any string.  VALUE may be empty.

=cut
sub EvaluateTerm ($)
{
    my $term = shift;

    if ($term =~ /^\s*([a-zA-Z_0-9]+)\s*(==|!=)\s*(.*)\s*$/)
    {
        my ($variable_name, $operator, $given_value) = ($1,$2,$3);
        my $variable_value = $ENV{$variable_name};
        $variable_value = "" if ! defined $variable_value;

        if ($operator eq "==")
        {
            return $variable_value eq $given_value;
        }
        elsif ($operator eq "!=")
        {
            return $variable_value ne $given_value;
        }
        else
        {
            die "unknown operator in term $term";
        }
    }
    elsif ($term =~ /^\s*true\s*$/i)
    {
        return 1;
    }
    elsif ($term =~ /^\s*false\s*$/i)
    {
        return 0;
    }
    else
    {
        die "term $term is not of the form <environment-variable> (=|==) <value>";
    }
}




=head IsPresent($name,$given_md5)

    Check if an external library tar ball with the basename $name already
    exists in the target directory TARFILE_LOCATION.  The basename is
    prefixed with the given MD5 sum.
    If the file exists then its MD5 sum is compare with the given MD5 sum.

=cut
sub IsPresent ($$)
{
    my $name = shift;
    my $given_md5 = shift;

    my $filename = File::Spec->catfile($ENV{'TARFILE_LOCATION'}, $given_md5."-".$name);

    return 0 if ! -f $filename;

    # File exists.  Check if its md5 sum is correct.
    my $md5 = Digest::MD5->new();
    open my $in, $filename;
    $md5->addfile($in);

    if ($given_md5 ne $md5->hexdigest())
    {
        # MD5 check sum does not match.  Delete the file.
        print "$name exists, but md5 does not match => deleting\n";
        #unlink($filename);
        return 0;
    }
    else
    {
        print "$name exists, md5 is OK\n";
        return 1;
    }
}




=head3 Download

    Download a set of files specified by @Missing.

    For http URLs there may be an optional MD5 checksum.  If it is present then downloaded
    files that do not match that checksum are an error and lead to abortion of the current process.
    Files that have already been downloaded are not downloaded again.

=cut
sub Download ()
{
    my $download_path = $ENV{'TARFILE_LOCATION'};

    if (scalar @Missing > 0)
    {
        printf("downloading %d missing tar ball%s to %s\n",
               scalar @Missing, scalar @Missing>0 ? "s" : "",
               $download_path);
    }
    else
    {
        print "all external libraries present\n";
        return;
    }

    # Download the missing files.
    for my $item (@Missing)
    {
        my ($name, $given_md5, $urls) = @$item;

        foreach my $url (@$urls)
        {
            last if DownloadFile($given_md5."-".$name, $url, $given_md5);
        }
    }
}




=head3 DownloadFile($name,$URL,$md5sum)

    Download a single external library tarball.  It origin is given by $URL.
    Its destination is $(TARFILE_LOCATION)/$md5sum-$name.

=cut
sub DownloadFile ($$$)
{
    my $name = shift;
    my $URL = shift;
    my $md5sum = shift;

    my $filename = File::Spec->catfile($ENV{'TARFILE_LOCATION'}, $name);

    my $temporary_filename = $filename . ".part";

    print "downloading to $temporary_filename\n";
    open my $out, ">$temporary_filename";
    binmode($out);

    # Prepare md5
    my $md5 = Digest::MD5->new();

    # Download the extension.
    my $agent = LWP::UserAgent->new();
    $agent->timeout(120);
    $agent->show_progress(1);
    my $last_was_redirect = 0;
    $agent->add_handler('response_redirect'
                        => sub{
                            $last_was_redirect = 1;
                            return;
                        });
    $agent->add_handler('response_data'
                        => sub{
                            if ($last_was_redirect)
                            {
                                $last_was_redirect = 0;
                                # Throw away the data we got so far.
                                $md5->reset();
                                close $out;
                                open $out, ">$temporary_filename";
                                binmode($out);
                            }
                            my($response,$agent,$h,$data)=@_;
                            print $out $data;
                            $md5->add($data);
                        });

    my $response = $agent->get($URL);
    close $out;

    # When download was successfull then check the md5 checksum and rename the .part file
    # into the actual extension name.
    if ($response->is_success())
    {
        my $file_md5 = $md5->hexdigest();
        if (defined $md5sum && length($md5sum)==32)
        {
            if ($md5sum eq $file_md5)
            {
                print "md5 is OK\n";
            }
            else
            {
                unlink($temporary_filename);
                print "    md5 does not match ($file_md5 instead of $md5sum)\n";
                return 0;
            }
        }
        else
        {
            printf("md5 not given, md5 of file is %s\n", $file_md5);
            $filename = File::Spec->catfile($ENV{'TARFILE_LOCATION'}, $file_md5 . "-" . $name);
        }

        rename($temporary_filename, $filename) || die "can not rename $temporary_filename to $filename";
        return 1;
    }
    else
    {
        unlink($temporary_filename);
        print "    download failed\n";
        return 0;
    }
}




=head3 CheckDownloadDestination ()

    Make sure that the download destination $TARFILE_LOCATION does exist.  If
    not, then the directory is created.

=cut
sub CheckDownloadDestination ()
{
    my $destination = $ENV{'TARFILE_LOCATION'};
    die "ERROR: no destination defined! please set TARFILE_LOCATION!" if ($destination eq "");

    if ( ! -d $destination)
    {
        File::Path::make_path($destination);
        die "ERROR: can't create \$TARFILE_LOCATION" if  ! -d $destination;
    }
}




=head3 ProvideSpecialTarball ($url,$name,$name_converter)

    A few tarballs need special handling.  That is done here.

=cut
sub ProvideSpecialTarball ($$$)
{
    my $url = shift;
    my $name = shift;
    my $name_converter = shift;

    return unless defined $url && $url ne "";

    # See if we can find the executable.
    my ($SOLARENV,$OUTPATH,$EXEEXT) =  ($ENV{'SOLARENV'},$ENV{'OUTPATH'},$ENV{'EXEEXT'});
    $SOLARENV = "" unless defined $SOLARENV;
    $OUTPATH = "" unless defined $OUTPATH;
    $EXEEXT = "" unless defined $EXEEXT;
    if (-x File::Spec->catfile($SOLARENV, $OUTPATH, "bin", $name.$EXEEXT))
    {
        print "found $name executable\n";
        return;
    }

    # Download the source from the URL.
    my $basename = basename(URI->new($url)->path());
    die unless defined $basename;

    if (defined $name_converter)
    {
        $basename = &{$name_converter}($basename);
    }

    # Has the source tar ball already been downloaded?
    my @candidates = glob(File::Spec->catfile($ENV{'TARFILE_LOCATION'}, "*-" . $basename));
    if (scalar @candidates > 0)
    {
        # Yes.
        print "$basename exists\n";
        return;
    }
    else
    {
        # No, download it.
        print "downloading $basename\n";
        DownloadFile($basename, $url, undef);
    }
}





# The main() functionality.

die "usage: $0 <data-file-name>" if scalar @ARGV != 1;
my $data_file = $ARGV[0];
CheckDownloadDestination();
ProcessDataFile($data_file);
ProvideSpecialTarball($ENV{'DMAKE_URL'}, "dmake", undef);
ProvideSpecialTarball(
    $ENV{'EPM_URL'},
    "epm",
    sub{$_[0]=~s/-source//; return $_[0]});
