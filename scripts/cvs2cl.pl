#! /usr/bin/perl -w


##############################################################
###                                                        ###
### cvs2cl.pl: produce ChangeLog(s) from `cvs log` output. ###
###                                                        ###
##############################################################

## $Revision: 2.57 $
## $Date: 2004/07/10 19:38:37 $
## $Author: fluffy $
##

use strict;

use File::Basename qw( fileparse );
use Getopt::Long   qw( GetOptions );
use Text::Wrap     qw( );
use User::pwent    qw( getpwnam );
use Time::Local    qw( timegm );

# The Plan:
#
# Read in the logs for multiple files, spit out a nice ChangeLog that
# mirrors the information entered during `cvs commit'.
#
# The problem presents some challenges. In an ideal world, we could
# detect files with the same author, log message, and checkin time --
# each <filelist, author, time, logmessage> would be a changelog entry.
# We'd sort them; and spit them out.  Unfortunately, CVS is *not atomic*
# so checkins can span a range of times.  Also, the directory structure
# could be hierarchical.
#
# Another question is whether we really want to have the ChangeLog
# exactly reflect commits. An author could issue two related commits,
# with different log entries, reflecting a single logical change to the
# source. GNU style ChangeLogs group these under a single author/date.
# We try to do the same.
#
# So, we parse the output of `cvs log', storing log messages in a
# multilevel hash that stores the mapping:
#   directory => author => time => message => filelist
# As we go, we notice "nearby" commit times and store them together
# (i.e., under the same timestamp), so they appear in the same log
# entry.
#
# When we've read all the logs, we twist this mapping into
# a time => author => message => filelist mapping for each directory.
#
# If we're not using the `--distributed' flag, the directory is always
# considered to be `./', even as descend into subdirectories.

# Call Tree

# name                         number of lines (10.xii.03)
# parse_options                         192
# derive_changelog                       13
# +-maybe_grab_accumulation_date         38
# +-read_changelog                      277
#   +-maybe_read_user_map_file           94
#     +-run_ext                           9
#   +-read_file_path                     29
#   +-read_symbolic_name                 43
#   +-read_revision                      49
#   +-read_date_author_and_state         25
#     +-parse_date_author_and_state      20
#   +-read_branches                      36
# +-output_changelog                    424
#   +-pretty_file_list                  290
#     +-common_path_prefix               35
#   +-preprocess_msg_text                30
#     +-min                               1
#   +-mywrap                             16
#   +-last_line_len                       5
#   +-wrap_log_entry                    177
#
# Utilities
#
# xml_escape                              6
# slurp_file                             11
# debug                                   5
# version                                 2
# usage                                 142

# -*- -*- -*- -*- -*- -*- -*- -*- -*- -*- -*- -*- -*- -*- -*- -*- -*- -*-
#
# Note about a bug-slash-opportunity:
# -----------------------------------
#
# There's a bug in Text::Wrap, which affects cvs2cl.  This script
# reveals it:
#
#   #!/usr/bin/perl -w
#
#   use Text::Wrap;
#
#   my $test_text =
#   "This script demonstrates a bug in Text::Wrap.  The very long line
#   following this paragraph will be relocated relative to the surrounding
#   text:
#
#   ====================================================================
#
#   See?  When the bug happens, we'll get the line of equal signs below
#   this paragraph, even though it should be above.";
#
#
#   # Print out the test text with no wrapping:
#   print "$test_text";
#   print "\n";
#   print "\n";
#
#   # Now print it out wrapped, and see the bug:
#   print wrap ("\t", "        ", "$test_text");
#   print "\n";
#   print "\n";
#
# If the line of equal signs were one shorter, then the bug doesn't
# happen.  Interesting.
#
# Anyway, rather than fix this in Text::Wrap, we might as well write a
# new wrap() which has the following much-needed features:
#
# * initial indentation, like current Text::Wrap()
# * subsequent line indentation, like current Text::Wrap()
# * user chooses among: force-break long words, leave them alone, or die()?
# * preserve existing indentation: chopped chunks from an indented line
#   are indented by same (like this line, not counting the asterisk!)
# * optional list of things to preserve on line starts, default ">"
#
# Note that the last two are essentially the same concept, so unify in
# implementation and give a good interface to controlling them.
#
# And how about:
#
# Optionally, when encounter a line pre-indented by same as previous
# line, then strip the newline and refill, but indent by the same.
# Yeah...

# Globals --------------------------------------------------------------------

# In case we have to print it out:
my $VERSION = '$Revision: 2.57 $';
$VERSION =~ s/\S+\s+(\S+)\s+\S+/$1/;

## Vars set by options:

# Print debugging messages?
my $Debug = 0;

# Just show version and exit?
my $Print_Version = 0;

# Just print usage message and exit?
my $Print_Usage = 0;

# What file should we generate (defaults to "ChangeLog")?
my $Log_File_Name = "ChangeLog";

# Grab most recent entry date from existing ChangeLog file, just add
# to that ChangeLog.
my $Cumulative = 0;

# `cvs log -d`, this will repeat the last entry in the old log.  This is OK,
# as it guarantees at least one entry in the update changelog, which means
# that there will always be a date to extract for the next update.  The repeat
# entry can be removed in postprocessing, if necessary.

# MJP 2003-08-02
# I don't think this actually does anything useful
my $Update = 0;

# Expand usernames to email addresses based on a map file?
my $User_Map_File = '';
my $User_Passwd_File;
my $Mail_Domain;

# Output log in chronological order? [default is reverse chronological order]
my $Chronological_Order = 0;

# Grab user details via gecos
my $Gecos = 0;

# User domain for gecos email addresses
my $Domain;

# Output to a file or to stdout?
my $Output_To_Stdout = 0;

# Eliminate empty log messages?
my $Prune_Empty_Msgs = 0;

# Tags of which not to output
my %ignore_tags;

# Show only revisions with Tags
my %show_tags;

# Don't call Text::Wrap on the body of the message
my $No_Wrap = 0;

# Indentation of log messages
my $Indent = "\t";

# Don't do any pretty print processing
my $Summary = 0;

# Separates header from log message.  Code assumes it is either " " or
# "\n\n", so if there's ever an option to set it to something else,
# make sure to go through all conditionals that use this var.
my $After_Header = " ";

# XML Encoding
my $XML_Encoding = '';

# Format more for programs than for humans.
my $XML_Output = 0;
my $No_XML_Namespace = 0;
my $No_XML_ISO_Date = 0;

# Do some special tweaks for log data that was written in FSF
# ChangeLog style.
my $FSF_Style = 0;

# Show times in UTC instead of local time
my $UTC_Times = 0;

# Show times in output?
my $Show_Times = 1;

# Show day of week in output?
my $Show_Day_Of_Week = 0;

# Show revision numbers in output?
my $Show_Revisions = 0;

# Show dead files in output?
my $Show_Dead = 0;

# Hide dead trunk files which were created as a result of additions on a
# branch?
my $Hide_Branch_Additions = 1;

# Show tags (symbolic names) in output?
my $Show_Tags = 0;

# Show tags separately in output?
my $Show_Tag_Dates = 0;

# Show branches by symbolic name in output?
my $Show_Branches = 0;

# Show only revisions on these branches or their ancestors.
my @Follow_Branches;
# Show only revisions on these branches or their ancestors; ignore descendent
# branches.
my @Follow_Only;

# Don't bother with files matching this regexp.
my @Ignore_Files;

# How exactly we match entries.  We definitely want "o",
# and user might add "i" by using --case-insensitive option.
my $Case_Insensitive = 0;

# Maybe only show log messages matching a certain regular expression.
my $Regexp_Gate = '';

# Pass this global option string along to cvs, to the left of `log':
my $Global_Opts = '';

# Pass this option string along to the cvs log subcommand:
my $Command_Opts = '';

# Read log output from stdin instead of invoking cvs log?
my $Input_From_Stdin = 0;

# Don't show filenames in output.
my $Hide_Filenames = 0;

# Don't shorten directory names from filenames.
my $Common_Dir = 1;

# Max checkin duration. CVS checkin is not atomic, so we may have checkin
# times that span a range of time. We assume that checkins will last no
# longer than $Max_Checkin_Duration seconds, and that similarly, no
# checkins will happen from the same users with the same message less
# than $Max_Checkin_Duration seconds apart.
my $Max_Checkin_Duration = 180;

# What to put at the front of [each] ChangeLog.
my $ChangeLog_Header = '';

# Whether to enable 'delta' mode, and for what start/end tags.
my $Delta_Mode = 0;
my $Delta_From = '';
my $Delta_To = '';

my $TestCode;

# Whether to parse filenames from the RCS filename, and if so what
# prefix to strip.
my $RCS_Root;

# Whether to output information on the # of lines added and removed
# by each file modification.
my $Show_Lines_Modified = 0;

## end vars set by options.

# latest observed times for the start/end tags in delta mode
my $Delta_StartTime = 0;
my $Delta_EndTime = 0;

my $No_Ancestors = 0;

my $No_Extra_Indent = 0;

my $GroupWithinDate = 0;

# ----------------------------------------------------------------------------

package CVS::Utils::ChangeLog::EntrySet;

sub new {
  my $class = shift;
  my %self;
  bless \%self, $class;
}

# -------------------------------------

sub output_changelog {
  my $output_type = $XML_Output ? 'XML' : 'Text';
  my $output_class = "CVS::Utils::ChangeLog::EntrySet::Output::${output_type}";
  my $output = $output_class->new(follow_branches => \@Follow_Branches,
                                  follow_only     => \@Follow_Only,
                                  ignore_tags     => \%ignore_tags,
                                  show_tags       => \%show_tags,
                                 );
  $output->output_changelog(@_);
}

# -------------------------------------

sub add_fileentry {
  my ($self, $file_full_path, $time, $revision, $state, $lines,
      $branch_names, $branch_roots, $branch_numbers,
      $symbolic_names, $author, $msg_txt) = @_;

      my $qunk =
        CVS::Utils::ChangeLog::FileEntry->new($file_full_path, $time, $revision,
                                              $state, $lines,
                                              $branch_names, $branch_roots,
                                              $branch_numbers,
                                              $symbolic_names);

      # We might be including revision numbers and/or tags and/or
      # branch names in the output.  Most of the code from here to
      # loop-end deals with organizing these in qunk.

      unless ( $Hide_Branch_Additions
               and
               $msg_txt =~ /file .+ was initially added on branch \S+./ ) {
        # Add this file to the list
        # (We use many spoonfuls of autovivication magic. Hashes and arrays
        # will spring into existence if they aren't there already.)

        &main::debug ("(pushing log msg for ". $qunk->dir_key . $qunk->filename . ")\n");

        # Store with the files in this commit.  Later we'll loop through
        # again, making sure that revisions with the same log message
        # and nearby commit times are grouped together as one commit.
        $self->{$qunk->dir_key}{$author}{$time}{$msg_txt} =
          CVS::Utils::ChangeLog::Message->new($msg_txt)
              unless exists $self->{$qunk->dir_key}{$author}{$time}{$msg_txt};
        $self->{$qunk->dir_key}{$author}{$time}{$msg_txt}->add_fileentry($qunk);
      }

}

# ----------------------------------------------------------------------------

package CVS::Utils::ChangeLog::EntrySet::Output::Text;

use base qw( CVS::Utils::ChangeLog::EntrySet::Output );

use File::Basename qw( fileparse );

sub new {
  my $class = shift;
  my $self = $class->SUPER::new(@_);
}

# -------------------------------------

sub wday {
  my $self = shift; my $class = ref $self;
  my ($wday) = @_;

  return $Show_Day_Of_Week ? ' ' . $class->weekday_en($wday) : '';
}

# -------------------------------------

sub header_line {
  my $self = shift;
  my ($time, $author, $lastdate) = @_;

  my $header_line = '';

  my (undef,$min,$hour,$mday,$mon,$year,$wday)
    = $UTC_Times ? gmtime($time) : localtime($time);

  my $date = $self->fdatetime($time);

  if ($Show_Times) {
    $header_line =
      sprintf "%s  %s\n\n", $date, $author;
  } else {
    if ( ! defined $lastdate or $date ne $lastdate or ! $GroupWithinDate ) {
      if ( $GroupWithinDate ) {
        $header_line = "$date\n\n";
      } else {
        $header_line = "$date  $author\n\n";
      }
    } else {
      $header_line = '';
    }
  }
}

# -------------------------------------

sub preprocess_msg_text {
  my $self = shift;
  my ($text) = @_;

  $text = $self->SUPER::preprocess_msg_text($text);

  unless ( $No_Wrap ) {
    # Strip off lone newlines, but only for lines that don't begin with
    # whitespace or a mail-quoting character, since we want to preserve
    # that kind of formatting.  Also don't strip newlines that follow a
    # period; we handle those specially next.  And don't strip
    # newlines that precede an open paren.
    1 while $text =~ s/(^|\n)([^>\s].*[^.\n])\n([^>\n])/$1$2 $3/g;

    # If a newline follows a period, make sure that when we bring up the
    # bottom sentence, it begins with two spaces.
    1 while $text =~ s/(^|\n)([^>\s].*)\n([^>\n])/$1$2  $3/g;
  }

  return $text;
}

# -------------------------------------

# Here we take a bunch of qunks and convert them into printed
# summary that will include all the information the user asked for.
sub pretty_file_list {
  my $self = shift;

  return ''
    if $Hide_Filenames;

  my $qunksref = shift;

  my @filenames;
  my $beauty = '';          # The accumulating header string for this entry.
  my %non_unanimous_tags;   # Tags found in a proper subset of qunks
  my %unanimous_tags;       # Tags found in all qunks
  my %all_branches;         # Branches found in any qunk
  my $fbegun = 0;           # Did we begin printing filenames yet?

  my ($common_dir, $qunkrefs) =
    $self->_pretty_file_list(\(%unanimous_tags, %non_unanimous_tags, %all_branches), $qunksref);

  my @qunkrefs = @$qunkrefs;

  # Not XML output, so complexly compactify for chordate consumption.  At this
  # point we have enough global information about all the qunks to organize
  # them non-redundantly for output.

  if ($common_dir) {
    # Note that $common_dir still has its trailing slash
    $beauty .= "$common_dir: ";
  }

  if ($Show_Branches)
  {
    # For trailing revision numbers.
    my @brevisions;

    foreach my $branch (keys (%all_branches))
    {
      foreach my $qunkref (@qunkrefs)
      {
        if ((defined ($qunkref->branch))
            and ($qunkref->branch eq $branch))
        {
          if ($fbegun) {
            # kff todo: comma-delimited in XML too?  Sure.
            $beauty .= ", ";
          }
          else {
            $fbegun = 1;
          }
          my $fname = substr ($qunkref->filename, length ($common_dir));
          $beauty .= $fname;
          $qunkref->{'printed'} = 1;  # Just setting a mark bit, basically

          if ( $Show_Tags and defined $qunkref->tags ) {
            my @tags = grep ($non_unanimous_tags{$_}, @{$qunkref->tags});

            if (@tags) {
              $beauty .= " (tags: ";
              $beauty .= join (', ', @tags);
              $beauty .= ")";
            }
          }

          if ($Show_Revisions) {
            # Collect the revision numbers' last components, but don't
            # print them -- they'll get printed with the branch name
            # later.
            $qunkref->revision =~ /.+\.([\d]+)$/;
            push (@brevisions, $1);

            # todo: we're still collecting branch roots, but we're not
            # showing them anywhere.  If we do show them, it would be
            # nifty to just call them revision "0" on a the branch.
            # Yeah, that's the ticket.
          }
        }
      }
      $beauty .= " ($branch";
      if (@brevisions) {
        if ((scalar (@brevisions)) > 1) {
          $beauty .= ".[";
          $beauty .= (join (',', @brevisions));
          $beauty .= "]";
        }
        else {
          # Square brackets are spurious here, since there's no range to
          # encapsulate
          $beauty .= ".$brevisions[0]";
        }
      }
      $beauty .= ")";
    }
  }

  # Okay; any qunks that were done according to branch are taken care
  # of, and marked as printed.  Now print everyone else.

  my %fileinfo_printed;
  foreach my $qunkref (@qunkrefs)
  {
    next if (defined ($qunkref->{'printed'}));   # skip if already printed

    my $b = substr ($qunkref->filename, length ($common_dir));
    # todo: Shlomo's change was this:
    # $beauty .= substr ($qunkref->filename,
    #              (($common_dir eq "./") ? '' : length ($common_dir)));
    $qunkref->{'printed'} = 1;  # Set a mark bit.

    if ($Show_Revisions || $Show_Tags || $Show_Dead)
    {
      my $started_addendum = 0;

      if ($Show_Revisions) {
        $started_addendum = 1;
        $b .= " (";
        $b .= $qunkref->revision;
      }
      if ($Show_Dead && $qunkref->state =~ /dead/)
      {
        # Deliberately not using $started_addendum. Keeping it simple.
        $b .= "[DEAD]";
      }
      if ($Show_Tags && (defined $qunkref->tags)) {
        my @tags = grep ($non_unanimous_tags{$_}, @{$qunkref->tags});
        if ((scalar (@tags)) > 0) {
          if ($started_addendum) {
            $b .= ", ";
          }
          else {
            $b .= " (tags: ";
          }
          $b .= join (', ', @tags);
          $started_addendum = 1;
        }
      }
      if ($started_addendum) {
        $b .= ")";
      }
    }

    unless ( exists $fileinfo_printed{$b} ) {
      if ($fbegun) {
        $beauty .= ", ";
      } else {
        $fbegun = 1;
      }
      $beauty .= $b, $fileinfo_printed{$b} = 1;
    }
  }

  # Unanimous tags always come last.
  if ($Show_Tags && %unanimous_tags)
  {
    $beauty .= " (utags: ";
    $beauty .= join (', ', sort keys (%unanimous_tags));
    $beauty .= ")";
  }

  # todo: still have to take care of branch_roots?

  $beauty = "$beauty:";

  return $beauty;
}

# -------------------------------------

sub output_tagdate {
  my $self = shift;
  my ($fh, $time, $tag) = @_;

  my $fdatetime = $self->fdatetime($time);
  print $fh "$fdatetime  tag $tag\n\n";
  return;
}

# -------------------------------------

sub format_body {
  my $self = shift;
  my ($msg, $files, $qunklist) = @_;

  my $body;

  if ( $No_Wrap and ! $Summary ) {
    $msg = $self->preprocess_msg_text($msg);
    $files = $self->mywrap("\t", "\t  ", "* $files");
    $msg =~ s/\n(.+)/\n$Indent$1/g;
    unless ($After_Header eq " ") {
      $msg =~ s/^(.+)/$Indent$1/g;
    }
    if ( $Hide_Filenames ) {
      $body = $After_Header . $msg;
    } else {
      $body = $files . $After_Header . $msg;
    }
  } elsif ( $Summary ) {
    my ($filelist, $qunk);
    my (@DeletedQunks, @AddedQunks, @ChangedQunks);

    $msg = $self->preprocess_msg_text($msg);
    #
    #     Sort the files (qunks) according to the operation that was
    # performed.  Files which were added have no line change
    # indicator, whereas deleted files have state dead.
    #
    foreach $qunk ( @$qunklist ) {
      if ( "dead" eq $qunk->state) {
        push @DeletedQunks, $qunk;
      } elsif ( ! defined $qunk->lines ) {
        push @AddedQunks, $qunk;
      } else {
        push @ChangedQunks, $qunk;
      }
    }
    #
    #     The qunks list was  originally in tree search order.  Let's
    # get that back.  The lists, if they exist, will be reversed upon
    # processing.
    #

    #
    #     Now write the three sections onto $filelist
    #
    if ( @DeletedQunks ) {
      $filelist .= "\tDeleted:\n";
      foreach $qunk ( @DeletedQunks ) {
        $filelist .= "\t\t" . $qunk->filename;
        $filelist .= " (" . $qunk->revision . ")";
        $filelist .= "\n";
      }
      undef @DeletedQunks;
    }

    if ( @AddedQunks ) {
      $filelist .= "\tAdded:\n";
      foreach $qunk (@AddedQunks) {
        $filelist .= "\t\t" . $qunk->filename;
        $filelist .= " (" . $qunk->revision . ")";
        $filelist .= "\n";
      }
      undef @AddedQunks ;
    }

    if ( @ChangedQunks ) {
      $filelist .= "\tChanged:\n";
      foreach $qunk (@ChangedQunks) {
        $filelist .= "\t\t" . $qunk->filename;
        $filelist .= " (" . $qunk->revision . ")";
        $filelist .= ", \"" . $qunk->state . "\"";
        $filelist .= ", lines: " . $qunk->lines;
        $filelist .= "\n";
      }
      undef @ChangedQunks;
    }

    chomp $filelist;

    if ( $Hide_Filenames ) {
      $filelist = '';
    }

    $msg =~ s/\n(.*)/\n$Indent$1/g;
    unless ( $After_Header eq " " or $FSF_Style ) {
      $msg =~ s/^(.*)/$Indent$1/g;
    }

    unless ( $No_Wrap ) {
      if ( $FSF_Style ) {
        $msg = $self->wrap_log_entry($msg, '', 69, 69);
        chomp($msg);
        chomp($msg);
      } else {
        $msg = $self->mywrap('', $Indent, "$msg");
        $msg =~ s/[ \t]+\n/\n/g;
      }
    }

    $body = $filelist . $After_Header . $msg;
  } else {  # do wrapping, either FSF-style or regular
    my $latter_wrap = $No_Extra_Indent ? $Indent : "$Indent  ";

    if ( $FSF_Style ) {
      $files = $self->mywrap($Indent, $latter_wrap, "* $files");

      my $files_last_line_len = 0;
      if ( $After_Header eq " " ) {
        $files_last_line_len = $self->last_line_len($files);
        $files_last_line_len += 1;  # for $After_Header
      }

      $msg = $self->wrap_log_entry($msg, $latter_wrap, 69-$files_last_line_len, 69);
      $body = $files . $After_Header . $msg;
    } else {  # not FSF-style
      $msg = $self->preprocess_msg_text($msg);
      $body = $files . $After_Header . $msg;
      $body = $self->mywrap($Indent, $latter_wrap, "* $body");
      $body =~ s/[ \t]+\n/\n/g;
    }
  }

  return $body;
}

# ----------------------------------------------------------------------------

package CVS::Utils::ChangeLog::EntrySet::Output::XML;

use base qw( CVS::Utils::ChangeLog::EntrySet::Output );

use File::Basename qw( fileparse );

sub new {
  my $class = shift;
  my $self = $class->SUPER::new(@_);
}

# -------------------------------------

sub header_line {
  my $self = shift;
  my ($time, $author, $lastdate) = @_;

  my $header_line = '';

  my $isoDate;

  my ($y, $m, $d, $H, $M, $S) = (gmtime($time))[5,4,3,2,1,0];

  # Ideally, this would honor $UTC_Times and use +HH:MM syntax
  $isoDate = sprintf("%04d-%02d-%02dT%02d:%02d:%02dZ",
                     $y + 1900, $m + 1, $d, $H, $M, $S);

  my (undef,$min,$hour,$mday,$mon,$year,$wday)
    = $UTC_Times ? gmtime($time) : localtime($time);

  my $date = $self->fdatetime($time);
  $wday = $self->wday($wday);

  $header_line =
    sprintf ("<date>%4u-%02u-%02u</date>\n${wday}<time>%02u:%02u</time>\n",
             $year+1900, $mon+1, $mday, $hour, $min);
  $header_line .= "<isoDate>$isoDate</isoDate>\n"
    unless $No_XML_ISO_Date;
  $header_line .= sprintf("<author>%s</author>\n" , $author);
}

# -------------------------------------

sub wday {
  my $self = shift; my $class = ref $self;
  my ($wday) = @_;

  return '<weekday>' . $class->weekday_en($wday) . "</weekday>\n";
}

# -------------------------------------

sub escape {
  my $self = shift;

  my $txt = shift;
  $txt =~ s/&/&amp;/g;
  $txt =~ s/</&lt;/g;
  $txt =~ s/>/&gt;/g;
  return $txt;
}

# -------------------------------------

sub output_header {
  my $self = shift;
  my ($fh) = @_;

  my $encoding    =
    length $XML_Encoding ? qq'encoding="$XML_Encoding"' : '';
  my $version     = 'version="1.0"';
  my $declaration =
    sprintf '<?xml %s?>', join ' ', grep length, $version, $encoding;
  my $root        =
    $No_XML_Namespace ?
      '<changelog>'     :
        '<changelog xmlns="http://www.red-bean.com/xmlns/cvs2cl/">';
  print $fh "$declaration\n\n$root\n\n";
}

# -------------------------------------

sub output_footer {
  my $self = shift;
  my ($fh) = @_;

  print $fh "</changelog>\n";
}

# -------------------------------------

sub preprocess_msg_text {
  my $self = shift;
  my ($text) = @_;

  $text = $self->SUPER::preprocess_msg_text($text);

  $text = $self->escape($text);
  chomp $text;
  $text = "<msg>${text}</msg>\n";

  return $text;
}

# -------------------------------------

# Here we take a bunch of qunks and convert them into a printed
# summary that will include all the information the user asked for.
sub pretty_file_list {
  my $self = shift;
  my ($qunksref) = @_;

  my $beauty = '';          # The accumulating header string for this entry.
  my %non_unanimous_tags;   # Tags found in a proper subset of qunks
  my %unanimous_tags;       # Tags found in all qunks
  my %all_branches;         # Branches found in any qunk
  my $fbegun = 0;           # Did we begin printing filenames yet?

  my ($common_dir, $qunkrefs) =
    $self->_pretty_file_list(\(%unanimous_tags, %non_unanimous_tags, %all_branches),
      $qunksref);

  my @qunkrefs = @$qunkrefs;

  # If outputting XML, then our task is pretty simple, because we
  # don't have to detect common dir, common tags, branch prefixing,
  # etc.  We just output exactly what we have, and don't worry about
  # redundancy or readability.

  foreach my $qunkref (@qunkrefs)
  {
    my $filename    = $qunkref->filename;
    my $state       = $qunkref->state;
    my $revision    = $qunkref->revision;
    my $tags        = $qunkref->tags;
    my $branch      = $qunkref->branch;
    my $branchroots = $qunkref->roots;
    my $lines       = $qunkref->lines;

    $filename = $self->escape($filename);   # probably paranoia
    $revision = $self->escape($revision);   # definitely paranoia

    $beauty .= "<file>\n";
    $beauty .= "<name>${filename}</name>\n";
    $beauty .= "<cvsstate>${state}</cvsstate>\n";
    $beauty .= "<revision>${revision}</revision>\n";

    if ($Show_Lines_Modified
        && $lines && $lines =~ m/\+(\d+)\s+-(\d+)/) {
        $beauty .= "<linesadded>$1</linesadded>\n";
        $beauty .= "<linesremoved>$2</linesremoved>\n";
    }

    if ($branch) {
      $branch   = $self->escape($branch);     # more paranoia
      $beauty .= "<branch>${branch}</branch>\n";
    }
    foreach my $tag (@$tags) {
      $tag = $self->escape($tag);  # by now you're used to the paranoia
      $beauty .= "<tag>${tag}</tag>\n";
    }
    foreach my $root (@$branchroots) {
      $root = $self->escape($root);  # which is good, because it will continue
      $beauty .= "<branchroot>${root}</branchroot>\n";
    }
    $beauty .= "</file>\n";
  }

  # Theoretically, we could go home now.  But as long as we're here,
  # let's print out the common_dir and utags, as a convenience to
  # the receiver (after all, earlier code calculated that stuff
  # anyway, so we might as well take advantage of it).

  if ((scalar (keys (%unanimous_tags))) > 1) {
    foreach my $utag ((keys (%unanimous_tags))) {
      $utag = $self->escape($utag);   # the usual paranoia
      $beauty .= "<utag>${utag}</utag>\n";
    }
  }
  if ($common_dir) {
    $common_dir = $self->escape($common_dir);
    $beauty .= "<commondir>${common_dir}</commondir>\n";
  }

  # That's enough for XML, time to go home:
  return $beauty;
}

# -------------------------------------

sub output_tagdate {
  # NOT YET DONE
}

# -------------------------------------

sub output_entry {
  my $self = shift;
  my ($fh, $entry) = @_;
  print $fh "<entry>\n$entry</entry>\n\n";
}

# -------------------------------------

sub format_body {
  my $self = shift;
  my ($msg, $files, $qunklist) = @_;

  $msg = $self->preprocess_msg_text($msg);
  return $files . $msg;
}

# ----------------------------------------------------------------------------

package CVS::Utils::ChangeLog::EntrySet::Output;

use Carp           qw( croak );
use File::Basename qw( fileparse );

# Class Utility Functions -------------

{ # form closure

my @weekdays = (qw(Sunday Monday Tuesday Wednesday Thursday Friday Saturday));
sub weekday_en {
  my $class = shift;
  return $weekdays[$_[0]];
}

}

# -------------------------------------

sub new {
  my ($proto, %args) = @_;
  my $class = ref $proto || $proto;

  my $follow_branches = delete $args{follow_branches};
  my $follow_only     = delete $args{follow_only};
  my $ignore_tags     = delete $args{ignore_tags};
  my $show_tags       = delete $args{show_tags};
  die "Unrecognized arg to EntrySet::Output::new: '$_'\n"
    for keys %args;

  bless +{follow_branches => $follow_branches,
          follow_only     => $follow_only,
          show_tags       => $show_tags,
          ignore_tags     => $ignore_tags,
         }, $class;
}

# Abstract Subrs ----------------------

sub wday               { croak "Whoops.  Abtract method call (wday).\n" }
sub pretty_file_list   { croak "Whoops.  Abtract method call (pretty_file_list).\n" }
sub output_tagdate     { croak "Whoops.  Abtract method call (output_tagdate).\n" }
sub header_line        { croak "Whoops.  Abtract method call (header_line).\n" }

# Instance Subrs ----------------------

sub output_header { }

# -------------------------------------

sub output_entry {
  my $self = shift;
  my ($fh, $entry) = @_;
  print $fh "$entry\n";
}

# -------------------------------------

sub output_footer { }

# -------------------------------------

sub escape { return $_[1] }

# -------------------------------------

sub _revision_is_wanted {
  my ($self, $qunk) = @_;

  my ($revision, $branch_numbers) = @{$qunk}{qw( revision branch_numbers )};
  my $follow_branches = $self->{follow_branches};
  my $follow_only     = $self->{follow_only};

#print STDERR "IG: ", join(',', keys %{$self->{ignore_tags}}), "\n";
#print STDERR "IX: ", join(',', @{$qunk->{tags}}), "\n" if defined $qunk->{tags};
#print STDERR "IQ: ", join(',', keys %{$qunk->{branch_numbers}}), "\n" if defined $qunk->{branch_numbers};
#use Data::Dumper; print STDERR Dumper $qunk;

  for my $ignore_tag (keys %{$self->{ignore_tags}}) {
    return
      if defined $qunk->{tags} and grep $_ eq $ignore_tag, @{$qunk->{tags}};
  }

  if ( keys %{$self->{show_tags}} ) {
    for my $show_tag (keys %{$self->{show_tags}}) {
      return
        if ! defined $qunk->{tags} or ! grep $_ eq $show_tag, @{$qunk->{tags}};
    }
  }

  return 1
    unless @$follow_branches + @$follow_only; # no follow is follow all

  for my $x (map([$_, 1], @$follow_branches),
             map([$_, 0], @$follow_only    )) {
    my ($branch, $followsub) = @$x;

    # Special case for following trunk revisions
    return 1
      if $branch =~ /^trunk$/i and $revision =~ /^[0-9]+\.[0-9]+$/;

    if ( my $branch_number = $branch_numbers->{$branch} ) {
      # Are we on one of the follow branches or an ancestor of same?

      # If this revision is a prefix of the branch number, or possibly is less
      # in the minormost number, OR if this branch number is a prefix of the
      # revision, then yes.  Otherwise, no.

      # So below, we determine if any of those conditions are met.

      # Trivial case: is this revision on the branch?  (Compare this way to
      # avoid regexps that screw up Emacs indentation, argh.)
      if ( substr($revision, 0, (length($branch_number) + 1))
           eq
           ($branch_number . ".") ) {
        if ( $followsub ) {
          return 1;
        } elsif (length($revision) == length($branch_number)+2 ) {
          return 1;
        }
      } elsif ( length($branch_number) > length($revision)
                and
                $No_Ancestors ) {
        # Non-trivial case: check if rev is ancestral to branch

        # r_left still has the trailing "."
        my ($r_left, $r_end) = ($revision =~ /^((?:\d+\.)+)(\d+)$/);

        # b_left still has trailing "."
        # b_mid has no trailing "."
        my ($b_left, $b_mid) = ($branch_number =~ /^((?:\d+\.)+)(\d+)\.\d+$/);
        return 1
          if $r_left eq $b_left and $r_end <= $b_mid;
      }
    }
  }

  return;
}

# -------------------------------------

sub output_changelog {
my $self = shift; my $class = ref $self;
  my ($grand_poobah) = @_;
  ### Process each ChangeLog

  while (my ($dir,$authorhash) = each %$grand_poobah)
  {
    &main::debug ("DOING DIR: $dir\n");

    # Here we twist our hash around, from being
    #   author => time => message => filelist
    # in %$authorhash to
    #   time => author => message => filelist
    # in %changelog.
    #
    # This is also where we merge entries.  The algorithm proceeds
    # through the timeline of the changelog with a sliding window of
    # $Max_Checkin_Duration seconds; within that window, entries that
    # have the same log message are merged.
    #
    # (To save space, we zap %$authorhash after we've copied
    # everything out of it.)

    my %changelog;
    while (my ($author,$timehash) = each %$authorhash)
    {
      my %stamptime;
      foreach my $time (sort {$a <=> $b} (keys %$timehash))
      {
        my $msghash = $timehash->{$time};
        while (my ($msg,$qunklist) = each %$msghash)
        {
          my $stamptime = $stamptime{$msg};
          if ((defined $stamptime)
              and (($time - $stamptime) < $Max_Checkin_Duration)
              and (defined $changelog{$stamptime}{$author}{$msg}))
          {
            push(@{$changelog{$stamptime}{$author}{$msg}}, $qunklist->files);
          }
          else {
            $changelog{$time}{$author}{$msg} = $qunklist->files;
            $stamptime{$msg} = $time;
          }
        }
      }
    }
    undef (%$authorhash);

    ### Now we can write out the ChangeLog!

    my ($logfile_here, $logfile_bak, $tmpfile);
    my $lastdate;

    if (! $Output_To_Stdout) {
      $logfile_here =  $dir . $Log_File_Name;
      $logfile_here =~ s/^\.\/\//\//;   # fix any leading ".//" problem
      $tmpfile      = "${logfile_here}.cvs2cl$$.tmp";
      $logfile_bak  = "${logfile_here}.bak";

      open (LOG_OUT, ">$tmpfile") or die "Unable to open \"$tmpfile\"";
    }
    else {
      open (LOG_OUT, ">-") or die "Unable to open stdout for writing";
    }

    print LOG_OUT $ChangeLog_Header;

    my %tag_date_printed;

    $self->output_header(\*LOG_OUT);

    my @key_list = ();
    if($Chronological_Order) {
        @key_list = sort {$a <=> $b} (keys %changelog);
    } else {
        @key_list = sort {$b <=> $a} (keys %changelog);
    }
    foreach my $time (@key_list)
    {
      next if ($Delta_Mode &&
               (($time <= $Delta_StartTime) ||
                ($time > $Delta_EndTime && $Delta_EndTime)));

      # Set up the date/author line.
      # kff todo: do some more XML munging here, on the header
      # part of the entry:
      my (undef,$min,$hour,$mday,$mon,$year,$wday)
          = $UTC_Times ? gmtime($time) : localtime($time);

      $wday = $self->wday($wday);
      # XML output includes everything else, we might as well make
      # it always include Day Of Week too, for consistency.
      my $authorhash = $changelog{$time};
      if ($Show_Tag_Dates) {
        my %tags;
        while (my ($author,$mesghash) = each %$authorhash) {
          while (my ($msg,$qunk) = each %$mesghash) {
            foreach my $qunkref2 (@$qunk) {
              if (defined ($qunkref2->tags)) {
                foreach my $tag (@{$qunkref2->tags}) {
                  $tags{$tag} = 1;
                }
              }
            }
          }
        }
        # Sort here for determinism to ease testing
        foreach my $tag (sort keys %tags) {
          if ( ! defined $tag_date_printed{$tag} ) {
            $tag_date_printed{$tag} = $time;
            $self->output_tagdate(\*LOG_OUT, $time, $tag);
          }
        }
      }
      while (my ($author,$mesghash) = each %$authorhash)
      {
        # If XML, escape in outer loop to avoid compound quoting:
        $author = $self->escape($author);

      FOOBIE:
        # We sort here to enable predictable ordering for the testing porpoises
        for my $msg (sort keys %$mesghash)
        {
          my $qunklist = $mesghash->{$msg};

          my @qunklist =
            grep $self->_revision_is_wanted($_), @$qunklist;

          next FOOBIE unless @qunklist;

          my $files               = $self->pretty_file_list(\@qunklist);
          my $header_line;          # date and author
          my $wholething;           # $header_line + $body

          my $date = $self->fdatetime($time);
          $header_line = $self->header_line($time, $author, $lastdate);
          $lastdate = $date;

          $Text::Wrap::huge = 'overflow'
            if $Text::Wrap::VERSION >= 2001.0130;
          # Reshape the body according to user preferences.
          my $body = $self->format_body($msg, $files, \@qunklist);

          $body =~ s/[ \t]+\n/\n/g;
          $wholething = $header_line . $body;

          # One last check: make sure it passes the regexp test, if the
          # user asked for that.  We have to do it here, so that the
          # test can match against information in the header as well
          # as in the text of the log message.

          # How annoying to duplicate so much code just because I
          # can't figure out a way to evaluate scalars on the trailing
          # operator portion of a regular expression.  Grrr.
          if ($Case_Insensitive) {
            unless ( $Regexp_Gate and ( $wholething !~ /$Regexp_Gate/oi ) ) {
              $self->output_entry(\*LOG_OUT, $wholething);
            }
          }
          else {
            unless ( $Regexp_Gate and ( $wholething !~ /$Regexp_Gate/o ) ) {
              $self->output_entry(\*LOG_OUT, $wholething);
            }
          }
        }
      }
    }

    $self->output_footer(\*LOG_OUT);

    close (LOG_OUT);

    if ( ! $Output_To_Stdout ) {
      # If accumulating, append old data to new before renaming.  But
      # don't append the most recent entry, since it's already in the
      # new log due to CVS's idiosyncratic interpretation of "log -d".
      if ($Cumulative && -f $logfile_here) {
        open NEW_LOG, ">>$tmpfile"
          or die "trouble appending to $tmpfile ($!)";

        open OLD_LOG, "<$logfile_here"
          or die "trouble reading from $logfile_here ($!)";

        my $started_first_entry = 0;
        my $passed_first_entry = 0;
        while (<OLD_LOG>) {
          if ( ! $passed_first_entry ) {
            if ( ( ! $started_first_entry )
                and /^(\d\d\d\d-\d\d-\d\d\s+\d\d:\d\d)/ ) {
              $started_first_entry = 1;
            } elsif ( /^(\d\d\d\d-\d\d-\d\d\s+\d\d:\d\d)/ ) {
              $passed_first_entry = 1;
              print NEW_LOG $_;
            }
          } else {
            print NEW_LOG $_;
          }
        }

        close NEW_LOG;
        close OLD_LOG;
      }

      if ( -f $logfile_here ) {
        rename $logfile_here, $logfile_bak;
      }
      rename $tmpfile, $logfile_here;
    }
  }
}

# -------------------------------------

# Don't call this wrap, because with 5.5.3, that clashes with the
# (unconditional :-( ) export of wrap() from Text::Wrap
sub mywrap {
  my $self = shift;
  my ($indent1, $indent2, @text) = @_;
  # If incoming text looks preformatted, don't get clever
  my $text = Text::Wrap::wrap($indent1, $indent2, @text);
  if ( grep /^\s+/m, @text ) {
    return $text;
  }
  my @lines = split /\n/, $text;
  $indent2 =~ s!^((?: {8})+)!"\t" x (length($1)/8)!e;
  $lines[0] =~ s/^$indent1\s+/$indent1/;
  s/^$indent2\s+/$indent2/
    for @lines[1..$#lines];
  my $newtext = join "\n", @lines;
  $newtext .= "\n"
    if substr($text, -1) eq "\n";
  return $newtext;
}

# -------------------------------------

sub preprocess_msg_text {
  my $self = shift;
  my ($text) = @_;

  # Strip out carriage returns (as they probably result from DOSsy editors).
  $text =~ s/\r\n/\n/g;
  # If it *looks* like two newlines, make it *be* two newlines:
  $text =~ s/\n\s*\n/\n\n/g;

  return $text;
}

# -------------------------------------

sub last_line_len {
  my $self = shift;

  my $files_list = shift;
  my @lines = split (/\n/, $files_list);
  my $last_line = pop (@lines);
  return length ($last_line);
}

# -------------------------------------

# A custom wrap function, sensitive to some common constructs used in
# log entries.
sub wrap_log_entry {
  my $self = shift;

  my $text = shift;                  # The text to wrap.
  my $left_pad_str = shift;          # String to pad with on the left.

  # These do NOT take left_pad_str into account:
  my $length_remaining = shift;      # Amount left on current line.
  my $max_line_length  = shift;      # Amount left for a blank line.

  my $wrapped_text = '';             # The accumulating wrapped entry.
  my $user_indent = '';              # Inherited user_indent from prev line.

  my $first_time = 1;                # First iteration of the loop?
  my $suppress_line_start_match = 0; # Set to disable line start checks.

  my @lines = split (/\n/, $text);
  while (@lines)   # Don't use `foreach' here, it won't work.
  {
    my $this_line = shift (@lines);
    chomp $this_line;

    if ($this_line =~ /^(\s+)/) {
      $user_indent = $1;
    }
    else {
      $user_indent = '';
    }

    # If it matches any of the line-start regexps, print a newline now...
    if ($suppress_line_start_match)
    {
      $suppress_line_start_match = 0;
    }
    elsif (($this_line =~ /^(\s*)\*\s+[a-zA-Z0-9]/)
           || ($this_line =~ /^(\s*)\* [a-zA-Z0-9_\.\/\+-]+/)
           || ($this_line =~ /^(\s*)\([a-zA-Z0-9_\.\/\+-]+(\)|,\s*)/)
           || ($this_line =~ /^(\s+)(\S+)/)
           || ($this_line =~ /^(\s*)- +/)
           || ($this_line =~ /^()\s*$/)
           || ($this_line =~ /^(\s*)\*\) +/)
           || ($this_line =~ /^(\s*)[a-zA-Z0-9](\)|\.|\:) +/))
    {
      # Make a line break immediately, unless header separator is set
      # and this line is the first line in the entry, in which case
      # we're getting the blank line for free already and shouldn't
      # add an extra one.
      unless (($After_Header ne " ") and ($first_time))
      {
        if ($this_line =~ /^()\s*$/) {
          $suppress_line_start_match = 1;
          $wrapped_text .= "\n${left_pad_str}";
        }

        $wrapped_text .= "\n${left_pad_str}";
      }

      $length_remaining = $max_line_length - (length ($user_indent));
    }

    # Now that any user_indent has been preserved, strip off leading
    # whitespace, so up-folding has no ugly side-effects.
    $this_line =~ s/^\s*//;

    # Accumulate the line, and adjust parameters for next line.
    my $this_len = length ($this_line);
    if ($this_len == 0)
    {
      # Blank lines should cancel any user_indent level.
      $user_indent = '';
      $length_remaining = $max_line_length;
    }
    elsif ($this_len >= $length_remaining) # Line too long, try breaking it.
    {
      # Walk backwards from the end.  At first acceptable spot, break
      # a new line.
      my $idx = $length_remaining - 1;
      if ($idx < 0) { $idx = 0 };
      while ($idx > 0)
      {
        if (substr ($this_line, $idx, 1) =~ /\s/)
        {
          my $line_now = substr ($this_line, 0, $idx);
          my $next_line = substr ($this_line, $idx);
          $this_line = $line_now;

          # Clean whitespace off the end.
          chomp $this_line;

          # The current line is ready to be printed.
          $this_line .= "\n${left_pad_str}";

          # Make sure the next line is allowed full room.
          $length_remaining = $max_line_length - (length ($user_indent));

          # Strip next_line, but then preserve any user_indent.
          $next_line =~ s/^\s*//;

          # Sneak a peek at the user_indent of the upcoming line, so
          # $next_line (which will now precede it) can inherit that
          # indent level.  Otherwise, use whatever user_indent level
          # we currently have, which might be none.
          my $next_next_line = shift (@lines);
          if ((defined ($next_next_line)) && ($next_next_line =~ /^(\s+)/)) {
            $next_line = $1 . $next_line if (defined ($1));
            # $length_remaining = $max_line_length - (length ($1));
            $next_next_line =~ s/^\s*//;
          }
          else {
            $next_line = $user_indent . $next_line;
          }
          if (defined ($next_next_line)) {
            unshift (@lines, $next_next_line);
          }
          unshift (@lines, $next_line);

          # Our new next line might, coincidentally, begin with one of
          # the line-start regexps, so we temporarily turn off
          # sensitivity to that until we're past the line.
          $suppress_line_start_match = 1;

          last;
        }
        else
        {
          $idx--;
        }
      }

      if ($idx == 0)
      {
        # We bottomed out because the line is longer than the
        # available space.  But that could be because the space is
        # small, or because the line is longer than even the maximum
        # possible space.  Handle both cases below.

        if ($length_remaining == ($max_line_length - (length ($user_indent))))
        {
          # The line is simply too long -- there is no hope of ever
          # breaking it nicely, so just insert it verbatim, with
          # appropriate padding.
          $this_line = "\n${left_pad_str}${this_line}";
        }
        else
        {
          # Can't break it here, but may be able to on the next round...
          unshift (@lines, $this_line);
          $length_remaining = $max_line_length - (length ($user_indent));
          $this_line = "\n${left_pad_str}";
        }
      }
    }
    else  # $this_len < $length_remaining, so tack on what we can.
    {
      # Leave a note for the next iteration.
      $length_remaining = $length_remaining - $this_len;

      if ($this_line =~ /\.$/)
      {
        $this_line .= "  ";
        $length_remaining -= 2;
      }
      else  # not a sentence end
      {
        $this_line .= " ";
        $length_remaining -= 1;
      }
    }

    # Unconditionally indicate that loop has run at least once.
    $first_time = 0;

    $wrapped_text .= "${user_indent}${this_line}";
  }

  # One last bit of padding.
  $wrapped_text .= "\n";

  return $wrapped_text;
}

# -------------------------------------

sub _pretty_file_list {
  my $self = shift;

  my ($unanimous_tags, $non_unanimous_tags, $all_branches, $qunksref) = @_;

  my @qunkrefs =
    grep +( ( ! $_->tags_exists
              or
              ! grep exists $ignore_tags{$_}, @{$_->tags})
            and
            ( ! keys %show_tags
              or
              ( $_->tags_exists
                and
                grep exists $show_tags{$_}, @{$_->tags} )
            )
          ),
    @$qunksref;

  my $common_dir;           # Dir prefix common to all files ('' if none)

  # First, loop over the qunks gathering all the tag/branch names.
  # We'll put them all in non_unanimous_tags, and take out the
  # unanimous ones later.
 QUNKREF:
  foreach my $qunkref (@qunkrefs)
  {
    # Keep track of whether all the files in this commit were in the
    # same directory, and memorize it if so.  We can make the output a
    # little more compact by mentioning the directory only once.
    if ($Common_Dir && (scalar (@qunkrefs)) > 1)
    {
      if (! (defined ($common_dir)))
      {
        my ($base, $dir);
        ($base, $dir, undef) = fileparse ($qunkref->filename);

        if ((! (defined ($dir)))  # this first case is sheer paranoia
            or ($dir eq '')
            or ($dir eq "./")
            or ($dir eq ".\\"))
        {
          $common_dir = '';
        }
        else
        {
          $common_dir = $dir;
        }
      }
      elsif ($common_dir ne '')
      {
        # Already have a common dir prefix, so how much of it can we preserve?
        $common_dir = &main::common_path_prefix ($qunkref->filename, $common_dir);
      }
    }
    else  # only one file in this entry anyway, so common dir not an issue
    {
      $common_dir = '';
    }

    if (defined ($qunkref->branch)) {
      $all_branches->{$qunkref->branch} = 1;
    }
    if (defined ($qunkref->tags)) {
      foreach my $tag (@{$qunkref->tags}) {
        $non_unanimous_tags->{$tag} = 1;
      }
    }
  }

  # Any tag held by all qunks will be printed specially... but only if
  # there are multiple qunks in the first place!
  if ((scalar (@qunkrefs)) > 1) {
    foreach my $tag (keys (%$non_unanimous_tags)) {
      my $everyone_has_this_tag = 1;
      foreach my $qunkref (@qunkrefs) {
        if ((! (defined ($qunkref->tags)))
            or (! (grep ($_ eq $tag, @{$qunkref->tags})))) {
          $everyone_has_this_tag = 0;
        }
      }
      if ($everyone_has_this_tag) {
        $unanimous_tags->{$tag} = 1;
        delete $non_unanimous_tags->{$tag};
      }
    }
  }

  return $common_dir, \@qunkrefs;
}

# -------------------------------------

sub fdatetime {
  my $self = shift;

  my ($year, $mday, $mon, $wday, $hour, $min);

  if ( @_ > 1 ) {
    ($year, $mday, $mon, $wday, $hour, $min) = @_;
  } else {
    my ($time) = @_;
    (undef, $min, $hour, $mday, $mon, $year, $wday) =
      $UTC_Times ? gmtime($time) : localtime($time);

    $year += 1900;
    $mon  += 1;
    $wday  = $self->wday($wday);
  }

  my $fdate = $self->fdate($year, $mon, $mday, $wday);

  if ($Show_Times) {
    my $ftime = $self->ftime($hour, $min);
    return "$fdate $ftime";
  } else {
    return $fdate;
  }
}

# -------------------------------------

sub fdate {
  my $self = shift;

  my ($year, $mday, $mon, $wday);

  if ( @_ > 1 ) {
    ($year, $mon, $mday, $wday) = @_;
  } else {
    my ($time) = @_;
    (undef, undef, undef, $mday, $mon, $year, $wday) =
      $UTC_Times ? gmtime($time) : localtime($time);

    $year += 1900;
    $mon  += 1;
    $wday  = $self->wday($wday);
  }

  return sprintf '%4u-%02u-%02u%s', $year, $mon, $mday, $wday;
}

# -------------------------------------

sub ftime {
  my $self = shift;

  my ($hour, $min);

  if ( @_ > 1 ) {
    ($hour, $min) = @_;
  } else {
    my ($time) = @_;
    (undef, $min, $hour) = $UTC_Times ? gmtime($time) : localtime($time);
  }

  return sprintf '%02u:%02u', $hour, $min;
}

# ----------------------------------------------------------------------------

package CVS::Utils::ChangeLog::Message;

sub new {
  my $class = shift;
  my ($msg) = @_;

  my %self = (msg => $msg, files => []);

  bless \%self, $class;
}

sub add_fileentry {
  my $self = shift;
  my ($fileentry) = @_;

  die "Not a fileentry: $fileentry"
    unless $fileentry->isa('CVS::Utils::ChangeLog::FileEntry');

  push @{$self->{files}}, $fileentry;
}

sub files { wantarray ? @{$_[0]->{files}} : $_[0]->{files} }

# ----------------------------------------------------------------------------

package CVS::Utils::ChangeLog::FileEntry;

use File::Basename qw( fileparse );

# Each revision of a file has a little data structure (a `qunk')
# associated with it.  That data structure holds not only the
# file's name, but any additional information about the file
# that might be needed in the output, such as the revision
# number, tags, branches, etc.  The reason to have these things
# arranged in a data structure, instead of just appending them
# textually to the file's name, is that we may want to do a
# little rearranging later as we write the output.  For example,
# all the files on a given tag/branch will go together, followed
# by the tag in parentheses (so trunk or otherwise non-tagged
# files would go at the end of the file list for a given log
# message).  This rearrangement is a lot easier to do if we
# don't have to reparse the text.
#
# A qunk looks like this:
#
#   {
#     filename    =>    "hello.c",
#     revision    =>    "1.4.3.2",
#     time        =>    a timegm() return value (moment of commit)
#     tags        =>    [ "tag1", "tag2", ... ],
#     branch      =>    "branchname" # There should be only one, right?
#     roots       =>    [ "branchtag1", "branchtag2", ... ]
#     lines       =>    "+x -y" # or undefined; x and y are integers
#   }

# Single top-level ChangeLog, or one per subdirectory?
my $distributed;
sub distributed { $#_ ? ($distributed = $_[1]) : $distributed; }

sub new {
  my $class = shift;
  my ($path, $time, $revision, $state, $lines,
      $branch_names, $branch_roots, $branch_numbers, $symbolic_names) = @_;

  my %self = (time     => $time,
              revision => $revision,
              state    => $state,
              lines    => $lines,
              branch_numbers => $branch_numbers,
             );

  if ( $distributed ) {
    @self{qw(filename dir_key)} = fileparse($path);
  } else {
    @self{qw(filename dir_key)} = ($path, './');
  }

  { # Scope for $branch_prefix
    (my ($branch_prefix) = ($revision =~ /((?:\d+\.)+)\d+/));
    $branch_prefix =~ s/\.$//;
    if ( $branch_names->{$branch_prefix} ) {
      my $branch_name = $branch_names->{$branch_prefix};
      $self{branch}   = $branch_name;
      $self{branches} = [$branch_name];
    }
    while ( $branch_prefix =~ s/^(\d+(?:\.\d+\.\d+)+)\.\d+\.\d+$/$1/ ) {
      push @{$self{branches}}, $branch_names->{$branch_prefix}
        if exists $branch_names->{$branch_prefix};
    }
  }

  # If there's anything in the @branch_roots array, then this
  # revision is the root of at least one branch.  We'll display
  # them as branch names instead of revision numbers, the
  # substitution for which is done directly in the array:
  $self{'roots'} = [ map { $branch_names->{$_} } @$branch_roots ]
    if @$branch_roots;

  if ( exists $symbolic_names->{$revision} ) {
    $self{tags} = delete $symbolic_names->{$revision};
    &main::delta_check($time, $self{tags});
  }

  bless \%self, $class;
}

sub filename       { $_[0]->{filename}       }
sub dir_key        { $_[0]->{dir_key}        }
sub revision       { $_[0]->{revision}       }
sub branch         { $_[0]->{branch}         }
sub state          { $_[0]->{state}          }
sub lines          { $_[0]->{lines}          }
sub roots          { $_[0]->{roots}          }
sub branch_numbers { $_[0]->{branch_numbers} }

sub tags        { $_[0]->{tags}     }
sub tags_exists {
  exists $_[0]->{tags};
}

# This may someday be used in a more sophisticated calculation of what other
# files are involved in this commit.  For now, we don't use it much except for
# delta mode, because the common-commit-detection algorithm is hypothesized to
# be "good enough" as it stands.
sub time     { $_[0]->{time}     }

# ----------------------------------------------------------------------------

package CVS::Utils::ChangeLog::EntrySetBuilder;

use File::Basename qw( fileparse );
use Time::Local    qw( timegm );

use constant MAILNAME => "/etc/mailname";

# In 'cvs log' output, one long unbroken line of equal signs separates files:
use constant FILE_SEPARATOR => '=' x 77;# . "\n";
# In 'cvs log' output, a shorter line of dashes separates log messages within
# a file:
use constant REV_SEPARATOR  => '-' x 28;# . "\n";

use constant EMPTY_LOG_MESSAGE => '*** empty log message ***';

# -------------------------------------

sub new {
  my ($proto) = @_;
  my $class = ref $proto || $proto;

  my $poobah  = CVS::Utils::ChangeLog::EntrySet->new;
  my $self = bless +{ grand_poobah => $poobah }, $class;

  $self->clear_file;
  $self->maybe_read_user_map_file;
  return $self;
}

# -------------------------------------

sub clear_msg {
  my ($self) = @_;

  # Make way for the next message
  undef $self->{rev_msg};
  undef $self->{rev_time};
  undef $self->{rev_revision};
  undef $self->{rev_author};
  undef $self->{rev_state};
  undef $self->{lines};
  $self->{rev_branch_roots} = [];       # For showing which files are branch
                                        # ancestors.
  $self->{collecting_symbolic_names} = 0;
}

# -------------------------------------

sub clear_file {
  my ($self) = @_;
  $self->clear_msg;

  undef $self->{filename};
  $self->{branch_names}   = +{};        # We'll grab branch names while we're
                                        # at it.
  $self->{branch_numbers} = +{};        # Save some revisions for
                                        # @Follow_Branches
  $self->{symbolic_names} = +{};        # Where tag names get stored.
}

# -------------------------------------

sub grand_poobah { $_[0]->{grand_poobah} }

# -------------------------------------

sub read_changelog {
  my ($self, $command) = @_;

#  my $grand_poobah = CVS::Utils::ChangeLog::EntrySet->new;

  if (! $Input_From_Stdin) {
    my $Log_Source_Command = join(' ', @$command);
    &main::debug ("(run \"${Log_Source_Command}\")\n");
    open (LOG_SOURCE, "$Log_Source_Command |")
        or die "unable to run \"${Log_Source_Command}\"";
  }
  else {
    open (LOG_SOURCE, "-") or die "unable to open stdin for reading";
  }

  binmode LOG_SOURCE;

 XX_Log_Source:
  while (<LOG_SOURCE>) {
    chomp;
    s!\r$!!;

    # If on a new file and don't see filename, skip until we find it, and
    # when we find it, grab it.
    if ( ! defined $self->{filename} ) {
      $self->read_file_path($_);
    } elsif ( /^symbolic names:$/ ) {
      $self->{collecting_symbolic_names} = 1;
    } elsif ( $self->{collecting_symbolic_names} ) {
      $self->read_symbolic_name($_);
    } elsif ( $_ eq FILE_SEPARATOR and ! defined $self->{rev_revision} ) {
      $self->clear_file;
    } elsif ( ! defined $self->{rev_revision} ) {
        # If have file name, but not revision, and see revision, then grab
        # it.  (We collect unconditionally, even though we may or may not
        # ever use it.)
      $self->read_revision($_);
    } elsif ( ! defined $self->{rev_time} ) { # and /^date: /) {
      $self->read_date_author_and_state($_);
    } elsif ( /^branches:\s+(.*);$/ ) {
      $self->read_branches($1);
    } elsif ( ! ( $_ eq FILE_SEPARATOR or $_ eq REV_SEPARATOR ) ) {
      # If have file name, time, and author, then we're just grabbing
      # log message texts:
      $self->{rev_msg} .= $_ . "\n";   # Normally, just accumulate the message...
    } else {
      if ( ! $self->{rev_msg}
           or $self->{rev_msg} =~ /^\s*(\.\s*)?$/
           or index($self->{rev_msg}, EMPTY_LOG_MESSAGE) > -1 ) {
        # ... until a msg separator is encountered:
        # Ensure the message contains something:
        $self->clear_msg
          if $Prune_Empty_Msgs;
        $self->{rev_msg} = "[no log message]\n";
      }

      $self->add_file_entry;

      if ( $_ eq FILE_SEPARATOR ) {
        $self->clear_file;
      } else {
        $self->clear_msg;
      }
    }
  }

  close LOG_SOURCE
    or die sprintf("Problem reading log input (exit/signal/core: %d/%d/%d)\n",
                   $? >> 8, $? & 127, $? & 128);
  return;
}

# -------------------------------------

sub add_file_entry {
  $_[0]->grand_poobah->add_fileentry(@{$_[0]}{qw(filename rev_time rev_revision
                                                 rev_state lines branch_names
                                                 rev_branch_roots
                                                 branch_numbers
                                                 symbolic_names
                                                 rev_author rev_msg)});
}

# -------------------------------------

sub maybe_read_user_map_file {
  my ($self) = @_;

  my %expansions;
  my $User_Map_Input;

  if ($User_Map_File)
  {
    if ( $User_Map_File =~ m{^([-\w\@+=.,\/]+):([-\w\@+=.,\/:]+)} and
         !-f $User_Map_File )
    {
      my $rsh = (exists $ENV{'CVS_RSH'} ? $ENV{'CVS_RSH'} : 'ssh');
      $User_Map_Input = "$rsh $1 'cat $2' |";
      &main::debug ("(run \"${User_Map_Input}\")\n");
    }
    else
    {
      $User_Map_Input = "<$User_Map_File";
    }

    open (MAPFILE, $User_Map_Input)
        or die ("Unable to open $User_Map_File ($!)");

    while (<MAPFILE>)
    {
      next if /^\s*#/;  # Skip comment lines.
      next if not /:/;  # Skip lines without colons.

      # It is now safe to split on ':'.
      my ($username, $expansion) = split ':';
      chomp $expansion;
      $expansion =~ s/^'(.*)'$/$1/;
      $expansion =~ s/^"(.*)"$/$1/;

      # If it looks like the expansion has a real name already, then
      # we toss the username we got from CVS log.  Otherwise, keep
      # it to use in combination with the email address.

      if ($expansion =~ /^\s*<{0,1}\S+@.*/) {
        # Also, add angle brackets if none present
        if (! ($expansion =~ /<\S+@\S+>/)) {
          $expansions{$username} = "$username <$expansion>";
        }
        else {
          $expansions{$username} = "$username $expansion";
        }
      }
      else {
        $expansions{$username} = $expansion;
      }
    } # fi ($User_Map_File)

    close (MAPFILE);
  }

  if (defined $User_Passwd_File)
  {
    if ( ! defined $Domain ) {
      if ( -e MAILNAME ) {
        chomp($Domain = slurp_file(MAILNAME));
      } else {
      MAILDOMAIN_CMD:
        for ([qw(hostname -d)], 'dnsdomainname', 'domainname') {
          my ($text, $exit, $sig, $core) = run_ext($_);
          if ( $exit == 0 && $sig == 0 && $core == 0 ) {
            chomp $text;
            if ( length $text ) {
              $Domain = $text;
              last MAILDOMAIN_CMD;
            }
          }
        }
      }
    }

    die "No mail domain found\n"
      unless defined $Domain;

    open (MAPFILE, "<$User_Passwd_File")
        or die ("Unable to open $User_Passwd_File ($!)");
    while (<MAPFILE>)
    {
      # all lines are valid
      my ($username, $pw, $uid, $gid, $gecos, $homedir, $shell) = split ':';
      my $expansion = '';
      ($expansion) = split (',', $gecos)
        if defined $gecos && length $gecos;

      my $mailname = $Domain eq '' ? $username : "$username\@$Domain";
      $expansions{$username} = "$expansion <$mailname>";
    }
    close (MAPFILE);
  }

 $self->{usermap} = \%expansions;
}

# -------------------------------------

sub read_file_path {
  my ($self, $line) = @_;

  my $path;

  if ( $line =~ /^Working file: (.*)/ ) {
    $path = $1;
  } elsif ( defined $RCS_Root
            and
            $line =~ m|^RCS file: $RCS_Root[/\\](.*),v$| ) {
    $path = $1;
    $path =~ s!Attic/!!;
  } else {
    return;
  }

  if ( @Ignore_Files ) {
    my $base;
    ($base, undef, undef) = fileparse($path);

    my $xpath = $Case_Insensitive ? lc($path) : $path;
    if ( grep index($path, $_) > -1, @Ignore_Files ) {
      return;
    }
  }

  $self->{filename} = $path;
  return;
}

# -------------------------------------

sub read_symbolic_name {
  my ($self, $line) = @_;

  # All tag names are listed with whitespace in front in cvs log
  # output; so if see non-whitespace, then we're done collecting.
  if ( /^\S/ ) {
    $self->{collecting_symbolic_names} = 0;
    return;
  } else {
    # we're looking at a tag name, so parse & store it

    # According to the Cederqvist manual, in node "Tags", tag names must start
    # with an uppercase or lowercase letter and can contain uppercase and
    # lowercase letters, digits, `-', and `_'.  However, it's not our place to
    # enforce that, so we'll allow anything CVS hands us to be a tag:
    my ($tag_name, $tag_rev) = ($line =~ /^\s+([^:]+): ([\d.]+)$/);

    # A branch number either has an odd number of digit sections
    # (and hence an even number of dots), or has ".0." as the
    # second-to-last digit section.  Test for these conditions.
    my $real_branch_rev = '';
    if ( $tag_rev =~ /^(\d+\.\d+\.)+\d+$/             # Even number of dots...
         and
         $tag_rev !~ /^(1\.)+1$/ ) {                  # ...but not "1.[1.]1"
      $real_branch_rev = $tag_rev;
    } elsif ($tag_rev =~ /(\d+\.(\d+\.)+)0.(\d+)/) {  # Has ".0."
      $real_branch_rev = $1 . $3;
    }

    # If we got a branch, record its number.
    if ( $real_branch_rev ) {
      $self->{branch_names}->{$real_branch_rev} = $tag_name;
      $self->{branch_numbers}->{$tag_name} = $real_branch_rev;
    } else {
      # Else it's just a regular (non-branch) tag.
      push @{$self->{symbolic_names}->{$tag_rev}}, $tag_name;
    }
  }

  $self->{collecting_symbolic_names} = 1;
  return;
}

# -------------------------------------

sub read_revision {
  my ($self, $line) = @_;

  my ($revision) = ( $line =~ /^revision (\d+\.[\d.]+)/ );

  return
    unless $revision;

  $self->{rev_revision} = $revision;
  return;
}

# -------------------------------------

{ # Closure over %gecos_warned
my %gecos_warned;
sub read_date_author_and_state {
  my ($self, $line) = @_;

  my ($time, $author, $state) = $self->parse_date_author_and_state($line);

  if ( defined($self->{usermap}->{$author}) and $self->{usermap}->{$author} ) {
    $author = $self->{usermap}->{$author};
  } elsif ( defined $Domain or $Gecos == 1 ) {
    my $email = $author;
    $email = $author."@".$Domain
      if defined $Domain && $Domain ne '';

    my $pw = getpwnam($author);
    my ($fullname, $office, $workphone, $homephone, $gcos);
    if ( defined $pw ) {
      $gcos = (getpwnam($author))[6];
      ($fullname, $office, $workphone, $homephone) =
        split /\s*,\s*/, $gcos;
    } else {
      warn "Couldn't find gecos info for author '$author'\n"
        unless $gecos_warned{$author}++;
      $fullname = '';
    }
    for (grep defined, $fullname, $office, $workphone, $homephone) {
      s/&/ucfirst(lc($pw->name))/ge;
    }
    $author = $fullname . "  <" . $email . ">"
      if $fullname ne '';
  }

  $self->{rev_state}  = $state;
  $self->{rev_time}   = $time;
  $self->{rev_author} = $author;
  return;
}
}

# -------------------------------------

sub read_branches {
  # A "branches: ..." line here indicates that one or more branches
  # are rooted at this revision.  If we're showing branches, then we
  # want to show that fact as well, so we collect all the branches
  # that this is the latest ancestor of and store them in
  # $self->[rev_branch_roots}.  Just for reference, the format of the
  # line we're seeing at this point is:
  #
  #    branches:  1.5.2;  1.5.4;  ...;
  #
  # Okay, here goes:
  my ($self, $line) = @_;

  # Ugh.  This really bothers me.  Suppose we see a log entry
  # like this:
  #
  #    ----------------------------
  #    revision 1.1
  #    date: 1999/10/17 03:07:38;  author: jrandom;  state: Exp;
  #    branches:  1.1.2;
  #    Intended first line of log message begins here.
  #    ----------------------------
  #
  # The question is, how we can tell the difference between that
  # log message and a *two*-line log message whose first line is
  #
  #    "branches:  1.1.2;"
  #
  # See the problem?  The output of "cvs log" is inherently
  # ambiguous.
  #
  # For now, we punt: we liberally assume that people don't
  # write log messages like that, and just toss a "branches:"
  # line if we see it but are not showing branches.  I hope no
  # one ever loses real log data because of this.
  if ( $Show_Branches ) {
    $line =~ s/(1\.)+1;|(1\.)+1$//;  # ignore the trivial branch 1.1.1
    $self->{rev_branch_roots} = [split /;\s+/, $line]
      if length $line;
  }
}

# -------------------------------------

sub parse_date_author_and_state {
  my ($self, $line) = @_;
  # Parses the date/time and author out of a line like:
  #
  # date: 1999/02/19 23:29:05;  author: apharris;  state: Exp;
  #
  # or, in CVS 1.12.9:
  #
  # date: 2004-06-05 16:10:32 +0000; author: somebody; state: Exp;

  my ($year, $mon, $mday, $hours, $min, $secs, $utcOffset, $author, $state, $rest) =
    $line =~
      m!(\d+)[-/](\d+)[-/](\d+)\s+(\d+):(\d+):(\d+)(\s+[+-]\d{4})?;\s+
        author:\s+([^;]+);\s+state:\s+([^;]+);(.*)!x
    or  die "Couldn't parse date ``$line''";
  die "Bad date or Y2K issues"
    unless $year > 1969 and $year < 2258;
  # Kinda arbitrary, but useful as a sanity check
  my $time = timegm($secs, $min, $hours, $mday, $mon-1, $year-1900);
  if ( defined $utcOffset ) {
    my ($plusminus, $hour, $minute) = ($utcOffset =~ m/([+-])(\d\d)(\d\d)/);
    my $offset = (($hour * 60) + $minute) * 60 * ($plusminus eq '+' ? -1 : 1);
    $time += $offset;
  }
  if ( $rest =~ m!\s+lines:\s+(.*)! ) {
    $self->{lines} = $1;
  }

  return $time, $author, $state;
}

# Subrs ----------------------------------------------------------------------

package main;

sub delta_check {
  my ($time, $tags) = @_;

  # If we're in 'delta' mode, update the latest observed times for the
  # beginning and ending tags, and when we get around to printing output, we
  # will simply restrict ourselves to that timeframe...
  return
    unless $Delta_Mode;

  $Delta_StartTime = $time
    if $time > $Delta_StartTime and grep { $_ eq $Delta_From } @$tags;

  $Delta_EndTime = $time
    if $time > $Delta_EndTime and grep { $_ eq $Delta_To } @$tags;
}

sub run_ext {
  my ($cmd) = @_;
  $cmd = [$cmd]
    unless ref $cmd;
  local $" = ' ';
  my $out = qx"@$cmd 2>&1";
  my $rv  = $?;
  my ($sig, $core, $exit) = ($? & 127, $? & 128, $? >> 8);
  return $out, $exit, $sig, $core;
}

# -------------------------------------

# If accumulating, grab the boundary date from pre-existing ChangeLog.
sub maybe_grab_accumulation_date {
  if (! $Cumulative || $Update) {
    return '';
  }

  # else

  open (LOG, "$Log_File_Name")
      or die ("trouble opening $Log_File_Name for reading ($!)");

  my $boundary_date;
  while (<LOG>)
  {
    if (/^(\d\d\d\d-\d\d-\d\d\s+\d\d:\d\d)/)
    {
      $boundary_date = "$1";
      last;
    }
  }

  close (LOG);

  # convert time from utc to local timezone if the ChangeLog has
  # dates/times in utc
  if ($UTC_Times && $boundary_date)
  {
    # convert the utc time to a time value
    my ($year,$mon,$mday,$hour,$min) = $boundary_date =~
      m#(\d+)-(\d+)-(\d+)\s+(\d+):(\d+)#;
    my $time = timegm(0,$min,$hour,$mday,$mon-1,$year-1900);
    # print the timevalue in the local timezone
    my ($ignore,$wday);
    ($ignore,$min,$hour,$mday,$mon,$year,$wday) = localtime($time);
    $boundary_date=sprintf ("%4u-%02u-%02u %02u:%02u",
                            $year+1900,$mon+1,$mday,$hour,$min);
  }

  return $boundary_date;
}

# -------------------------------------

# Fills up a ChangeLog structure in the current directory.
sub derive_changelog {
  my ($command) = @_;

  # See "The Plan" above for a full explanation.

  # Might be adding to an existing ChangeLog
  my $accumulation_date = maybe_grab_accumulation_date;
  if ($accumulation_date) {
    # Insert -d immediately after 'cvs log'
    my $Log_Date_Command = "-d\'>${accumulation_date}\'";

    my ($log_index) = grep $command->[$_] eq 'log', 0..$#$command;
    splice @$command, $log_index+1, 0, $Log_Date_Command;
    &debug ("(adding log msg starting from $accumulation_date)\n");
  }

#  output_changelog(read_changelog($command));
  my $builder = CVS::Utils::ChangeLog::EntrySetBuilder->new;
  $builder->read_changelog($command);
  $builder->grand_poobah->output_changelog;
}

# -------------------------------------

sub min { $_[0] < $_[1] ? $_[0] : $_[1] }

# -------------------------------------

sub common_path_prefix {
  my ($path1, $path2) = @_;

  # For compatibility (with older versions of cvs2cl.pl), we think in UN*X
  # terms, and mould windoze filenames to match.  Is this really appropriate?
  # If a file is checked in under UN*X, and cvs log run on windoze, which way
  # do the path separators slope?  Can we use fileparse as per the local
  # conventions?  If so, we should probably have a user option to specify an
  # OS to emulate to handle stdin-fed logs.  If we did this, we could avoid
  # the nasty \-/ transmogrification below.

  my ($dir1, $dir2) = map +(fileparse($_))[1], $path1, $path2;

  # Transmogrify Windows filenames to look like Unix.
  # (It is far more likely that someone is running cvs2cl.pl under
  # Windows than that they would genuinely have backslashes in their
  # filenames.)
  tr!\\!/!
    for $dir1, $dir2;

  my ($accum1, $accum2, $last_common_prefix) = ('') x 3;

  my @path1 = grep length($_), split qr!/!, $dir1;
  my @path2 = grep length($_), split qr!/!, $dir2;

  my @common_path;
  for (0..min($#path1,$#path2)) {
    if ( $path1[$_] eq $path2[$_]) {
      push @common_path, $path1[$_];
    } else {
      last;
    }
  }

  return join '', map "$_/", @common_path;
}

# -------------------------------------
sub parse_options {
  # Check this internally before setting the global variable.
  my $output_file;

  # If this gets set, we encountered unknown options and will exit at
  # the end of this subroutine.
  my $exit_with_admonishment = 0;

  # command to generate the log
  my @log_source_command = qw( cvs log );

  my (@Global_Opts, @Local_Opts);

  Getopt::Long::Configure(qw( bundling permute no_getopt_compat
                              pass_through no_ignore_case ));
  GetOptions('help|usage|h'   => \$Print_Usage,
             'debug'          => \$Debug,        # unadvertised option, heh
             'version'        => \$Print_Version,

             'file|f=s'       => \$output_file,
             'accum'          => \$Cumulative,
             'update'         => \$Update,
             'fsf'            => \$FSF_Style,
             'rcs=s'          => \$RCS_Root,
             'usermap|U=s'    => \$User_Map_File,
             'gecos'          => \$Gecos,
             'domain=s'       => \$Domain,
             'passwd=s'       => \$User_Passwd_File,
             'window|W=i'     => \$Max_Checkin_Duration,
             'chrono'         => \$Chronological_Order,
             'ignore|I=s'     => \@Ignore_Files,
             'case-insensitive|C' => \$Case_Insensitive,
             'regexp|R=s'     => \$Regexp_Gate,
             'stdin'          => \$Input_From_Stdin,
             'stdout'         => \$Output_To_Stdout,
             'distributed|d'  => sub { CVS::Utils::ChangeLog::FileEntry->distributed(1) },
             'prune|P'        => \$Prune_Empty_Msgs,
             'no-wrap'        => \$No_Wrap,
             'gmt|utc'        => \$UTC_Times,
             'day-of-week|w'  => \$Show_Day_Of_Week,
             'revisions|r'    => \$Show_Revisions,
             'show-dead'      => \$Show_Dead,
             'tags|t'         => \$Show_Tags,
             'tagdates|T'     => \$Show_Tag_Dates,
             'branches|b'     => \$Show_Branches,
             'follow|F=s'     => \@Follow_Branches,
             'follow-only=s'  => \@Follow_Only,
             'xml-encoding=s' => \$XML_Encoding,
             'xml'            => \$XML_Output,
             'noxmlns'        => \$No_XML_Namespace,
             'no-xml-iso-date' => \$No_XML_ISO_Date,
             'no-ancestors'   => \$No_Ancestors,
             'lines-modified' => \$Show_Lines_Modified,

             'no-indent'    => sub {
               $Indent = '';
             },

             'summary'      => sub {
               $Summary = 1;
               $After_Header = "\n\n"; # Summary implies --separate-header
             },

             'no-times'     => sub {
               $Show_Times = 0;
             },

             'no-hide-branch-additions' => sub {
               $Hide_Branch_Additions = 0;
             },

             'no-common-dir'  => sub {
               $Common_Dir = 0;
             },

             'ignore-tag=s'   => sub {
               $ignore_tags{$_[1]} = 1;
             },

             'show-tag=s'     => sub {
               $show_tags{$_[1]} = 1;
             },

             # Deliberately undocumented.  This is not a public interface, and
             # may change/disappear at any time.
             'test-code=s'    => \$TestCode,

             'delta=s'        => sub {
               my $arg = $_[1];
               if ( $arg =~
                    /^([A-Za-z][A-Za-z0-9_\-\]\[]*):([A-Za-z][A-Za-z0-9_\-\]\[]*)$/ ) {
                 $Delta_From = $1;
                 $Delta_To = $2;
                 $Delta_Mode = 1;
               } else {
                 die "--delta FROM_TAG:TO_TAG is what you meant to say.\n";
               }
             },

             'FSF'             => sub {
               $Show_Times = 0;
               $Common_Dir = 0;
               $No_Extra_Indent = 1;
               $Indent = "\t";
             },

             'header=s'        => sub {
               my $narg = $_[1];
               $ChangeLog_Header = &slurp_file ($narg);
               if (! defined ($ChangeLog_Header)) {
                 $ChangeLog_Header = '';
               }
             },

             'global-opts|g=s' => sub {
               my $narg = $_[1];
               push @Global_Opts, $narg;
               splice @log_source_command, 1, 0, $narg;
             },

             'log-opts|l=s' => sub {
               my $narg = $_[1];
               push @Local_Opts, $narg;
               push @log_source_command, $narg;
             },

             'mailname=s'   => sub {
               my $narg = $_[1];
               warn "--mailname is deprecated; please use --domain instead\n";
               $Domain = $narg;
             },

             'separate-header|S' => sub {
               $After_Header = "\n\n";
               $No_Extra_Indent = 1;
             },

             'group-within-date' => sub {
               $GroupWithinDate = 1;
               $Show_Times = 0;
             },

             'hide-filenames' => sub {
               $Hide_Filenames = 1;
               $After_Header = '';
             },
            )
    or die "options parsing failed\n";

  push @log_source_command, map "'$_'", @ARGV;

  ## Check for contradictions...

  if ($Output_To_Stdout && CVS::Utils::ChangeLog::FileEntry->distributed) {
    print STDERR "cannot pass both --stdout and --distributed\n";
    $exit_with_admonishment = 1;
  }

  if ($Output_To_Stdout && $output_file) {
    print STDERR "cannot pass both --stdout and --file\n";
    $exit_with_admonishment = 1;
  }

  if ($Input_From_Stdin && @Global_Opts) {
    print STDERR "cannot pass both --stdin and -g\n";
    $exit_with_admonishment = 1;
  }

  if ($Input_From_Stdin && @Local_Opts) {
    print STDERR "cannot pass both --stdin and -l\n";
    $exit_with_admonishment = 1;
  }

  if ($XML_Output && $Cumulative) {
    print STDERR "cannot pass both --xml and --accum\n";
    $exit_with_admonishment = 1;
  }

  # Other consistency checks and option-driven logic

  # Bleargh.  Compensate for a deficiency of custom wrapping.
  if ( ($After_Header ne " ") and $FSF_Style ) {
    $After_Header .= "\t";
  }

  @Ignore_Files = map lc, @Ignore_Files
    if $Case_Insensitive;

  # Or if any other error message has already been printed out, we
  # just leave now:
  if ($exit_with_admonishment) {
    &usage ();
    exit (1);
  }
  elsif ($Print_Usage) {
    &usage ();
    exit (0);
  }
  elsif ($Print_Version) {
    &version ();
    exit (0);
  }

  ## Else no problems, so proceed.

  if ($output_file) {
    $Log_File_Name = $output_file;
  }

  return \@log_source_command;
}

# -------------------------------------

sub slurp_file {
  my $filename = shift || die ("no filename passed to slurp_file()");
  my $retstr;

  open (SLURPEE, "<${filename}") or die ("unable to open $filename ($!)");
  local $/ = undef;
  $retstr = <SLURPEE>;
  close (SLURPEE);
  return $retstr;
}

# -------------------------------------

sub debug {
  if ($Debug) {
    my $msg = shift;
    print STDERR $msg;
  }
}

# -------------------------------------

sub version {
  print "cvs2cl.pl version ${VERSION}; distributed under the GNU GPL.\n";
}

# -------------------------------------

sub usage {
  &version ();

  eval "use Pod::Usage qw( pod2usage )";

   if ( $@ ) {
    print <<'END';

* Pod::Usage was not found.  The formatting may be suboptimal.  Consider
  upgrading your Perl --- Pod::Usage is standard from 5.6 onwards, and
  versions of perl prior to 5.6 are getting rather rusty, now.  Alternatively,
  install Pod::Usage direct from CPAN.
END

    local $/ = undef;
    my $message = <DATA>;
    $message =~ s/^=(head1|item) //gm;
    $message =~ s/^=(over|back).*\n//gm;
    $message =~ s/\n{3,}/\n\n/g;
    print $message;
  } else {
    print "\n";
    pod2usage( -exitval => 'NOEXIT',
               -verbose => 1,
               -output  => \*STDOUT,
             );
  }

  return;
}

# Main -----------------------------------------------------------------------

my $log_source_command = parse_options;
if ( defined $TestCode ) {
  eval $TestCode;
  die "Eval failed: '$@'\n"
    if $@;
} else {
  derive_changelog($log_source_command);
}

__DATA__

=head1 NAME

cvs2cl.pl - convert cvs log messages to changelogs

=head1 SYNOPSIS

B<cvs2cl> [I<options>] [I<FILE1> [I<FILE2> ...]]

=head1 DESCRIPTION

cvs2cl produces a GNU-style ChangeLog for CVS-controlled sources by
running "cvs log" and parsing the output. Duplicate log messages get
unified in the Right Way.

The default output of cvs2cl is designed to be compact, formally unambiguous,
but still easy for humans to read.  It should be largely self-explanatory; the
one abbreviation that might not be obvious is "utags".  That stands for
"universal tags" -- a universal tag is one held by all the files in a given
change entry.

If you need output that's easy for a program to parse, use the B<--xml> option.
Note that with XML output, just about all available information is included
with each change entry, whether you asked for it or not, on the theory that
your parser can ignore anything it's not looking for.

If filenames are given as arguments cvs2cl only shows log information for the
named files.

=head1 OPTIONS

=over 4

=item B<-h>, B<-help>, B<--help>, B<-?>

Show a short help and exit.

=item B<--version>

Show version and exit.

=item B<-r>, B<--revisions>

Show revision numbers in output.

=item B<-b>, B<--branches>

Show branch names in revisions when possible.

=item B<-t>, B<--tags>

Show tags (symbolic names) in output.

=item B<-T>, B<--tagdates>

Show tags in output on their first occurance.

=item B<--show-dead>

Show dead files.

=item B<--stdin>

Read from stdin, don't run cvs log.

=item B<--stdout>

Output to stdout not to ChangeLog.

=item B<-d>, B<--distributed>

Put ChangeLogs in subdirs.

=item B<-f> I<FILE>, B<--file> I<FILE>

Write to I<FILE> instead of ChangeLog.

=item B<--fsf>

Use this if log data is in FSF ChangeLog style.

=item B<--FSF>

Attempt strict FSF-standard compatible output.

=item B<-W> I<SECS>, B<--window> I<SECS>

Window of time within which log entries unify.

=item -B<U> I<UFILE>, B<--usermap> I<UFILE>

Expand usernames to email addresses from I<UFILE>.

=item B<--passwd> I<PASSWORDFILE>

Use system passwd file for user name expansion.  If no mail domain is provided
(via B<--domain>), it tries to read one from B</etc/mailname>, output of B<hostname
-d>, B<dnsdomainname>, or B<domain-name>.  cvs2cl exits with an error if none of
those options is successful. Use a domain of '' to prevent the addition of a
mail domain.

=item B<--domain> I<DOMAIN>

Domain to build email addresses from.

=item B<--gecos>

Get user information from GECOS data.

=item B<-R> I<REGEXP>, B<--regexp> I<REGEXP>

Include only entries that match I<REGEXP>.  This option may be used multiple
times.

=item B<-I> I<REGEXP>, B<--ignore> I<REGEXP>

Ignore files whose names match I<REGEXP>.  This option may be used multiple
times.

=item B<-C>, B<--case-insensitive>

Any regexp matching is done case-insensitively.

=item B<-F> I<BRANCH>, B<--follow> I<BRANCH>

Show only revisions on or ancestral to I<BRANCH>.

=item B<--follow-only> I<BRANCH>

Like --follow, but sub-branches are not followed.

=item B<--no-ancestors>

When using B<-F>, only track changes since the I<BRANCH> started.

=item B<--no-hide-branch-additions>

By default, entries generated by cvs for a file added on a branch (a dead 1.1
entry) are not shown.  This flag reverses that action.

=item B<-S>, B<--separate-header>

Blank line between each header and log message.

=item B<--summary>

Add CVS change summary information.

=item B<--no-wrap>

Don't auto-wrap log message (recommend B<-S> also).

=item B<--no-indent>

Don't indent log message

=item B<--gmt>, B<--utc>

Show times in GMT/UTC instead of local time.

=item B<--accum>

Add to an existing ChangeLog (incompatible with B<--xml>).

=item B<-w>, B<--day-of-week>

Show day of week.

=item B<--no-times>

Don't show times in output.

=item B<--chrono>

Output log in chronological order (default is reverse chronological order).

=item B<--header> I<FILE>

Get ChangeLog header from I<FILE> ("B<->" means stdin).

=item B<--xml>

Output XML instead of ChangeLog format.

=item B<--xml-encoding> I<ENCODING.>

Insert encoding clause in XML header.

=item B<--noxmlns>

Don't include xmlns= attribute in root element.

=item B<--hide-filenames>

Don't show filenames (ignored for XML output).

=item B<--no-common-dir>

Don't shorten directory names from filenames.

=item B<--rcs> I<CVSROOT>

Handle filenames from raw RCS, for instance those produced by "cvs rlog"
output, stripping the prefix I<CVSROOT>.

=item B<-P>, B<--prune>

Don't show empty log messages.

=item B<--lines-modified>

Output the number of lines added and the number of lines removed for
each checkin (if applicable). At the moment, this only affects the
XML output mode.

=item B<--ignore-tag> I<TAG>

Ignore individual changes that are associated with a given tag.
May be repeated, if so, changes that are associated with any of
the given tags are ignored.

=item B<--show-tag> I<TAG>

Log only individual changes that are associated with a given
tag.  May be repeated, if so, changes that are associated with
any of the given tags are logged.

=item B<--delta> I<FROM_TAG>B<:>I<TO_TAG>

Attempt a delta between two tags (since I<FROM_TAG> up to and
including I<TO_TAG>).  The algorithm is a simple date-based one
(this is a hard problem) so results are imperfect.

=item B<-g> I<OPTS>, B<--global-opts> I<OPTS>

Pass I<OPTS> to cvs like in "cvs I<OPTS> log ...".

=item B<-l> I<OPTS>, B<--log-opts> I<OPTS>

Pass I<OPTS> to cvs log like in "cvs ... log I<OPTS>".

=back

Notes about the options and arguments:

=over 4

=item *

The B<-I> and B<-F> options may appear multiple times.

=item *

To follow trunk revisions, use "B<-F trunk>" ("B<-F TRUNK>" also works).  This is
okay because no would ever, ever be crazy enough to name a branch "trunk",
right?  Right.

=item *

For the B<-U> option, the I<UFILE> should be formatted like CVSROOT/users. That is,
each line of I<UFILE> looks like this:

       jrandom:jrandom@red-bean.com

or maybe even like this

       jrandom:'Jesse Q. Random <jrandom@red-bean.com>'

Don't forget to quote the portion after the colon if necessary.

=item *

Many people want to filter by date.  To do so, invoke cvs2cl.pl like this:

       cvs2cl.pl -l "-d'DATESPEC'"

where DATESPEC is any date specification valid for "cvs log -d".  (Note that
CVS 1.10.7 and below requires there be no space between -d and its argument).

=item *

Dates/times are interpreted in the local time zone.

=item *

Remember to quote the argument to `B<-l>' so that your shell doesn't interpret
spaces as argument separators.

=item *

See the 'Common Options' section of the cvs manual ('info cvs' on UNIX-like
systems) for more information.

=item *

Note that the rules for quoting under windows shells are different.

=back

=head1 EXAMPLES

Some examples (working on UNIX shells):

      # logs after 6th March, 2003 (inclusive)
      cvs2cl.pl -l "-d'>2003-03-06'"
      # logs after 4:34PM 6th March, 2003 (inclusive)
      cvs2cl.pl -l "-d'>2003-03-06 16:34'"
      # logs between 4:46PM 6th March, 2003 (exclusive) and
      # 4:34PM 6th March, 2003 (inclusive)
      cvs2cl.pl -l "-d'2003-03-06 16:46>2003-03-06 16:34'"

Some examples (on non-UNIX shells):

      # Reported to work on windows xp/2000
      cvs2cl.pl -l  "-d"">2003-10-18;today<"""

=head1 AUTHORS

=over 4

=item Karl Fogel

=item Melissa O'Neill

=item Martyn J. Pearce

=back

Contributions from

=over 4

=item Mike Ayers

=item Tim Bradshaw

=item Richard Broberg

=item Nathan Bryant

=item Oswald Buddenhagen

=item Neil Conway

=item Arthur de Jong

=item Mark W. Eichin

=item Dave Elcock

=item Reid Ellis

=item Simon Josefsson

=item Robin Hugh Johnson

=item Terry Kane

=item Akos Kiss

=item Claus Klein

=item Eddie Kohler

=item Richard Laager

=item Kevin Lilly

=item Karl-Heinz Marbaise

=item Mitsuaki Masuhara

=item Henrik Nordstrom

=item Joe Orton

=item Peter Palfrader

=item Thomas Parmelan

=item Johanne Stezenbach

=item Joseph Walton

=item Ernie Zapata

=back

=head1 BUGS

Please report bugs to C<bug-cvs2cl@red-bean.com>.

=head1 PREREQUISITES

This script requires C<Text::Wrap>, C<Time::Local>, and C<File::Basename>.  It
also seems to require C<Perl 5.004_04> or higher.

=head1 OPERATING SYSTEM COMPATIBILITY

Should work on any OS.

=head1 SCRIPT CATEGORIES

Version_Control/CVS

=head1 COPYRIGHT

(C) 2001,2002,2003,2004 Martyn J. Pearce <fluffy@cpan.org>, under the GNU GPL.

(C) 1999 Karl Fogel <kfogel@red-bean.com>, under the GNU GPL.

cvs2cl.pl is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

cvs2cl.pl is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You may have received a copy of the GNU General Public License
along with cvs2cl.pl; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

=head1 SEE ALSO

cvs(1)

