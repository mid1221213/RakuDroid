#! /usr/bin/env perl

use strict;
use warnings;
use utf8;
use feature 'say';

my %classes;
my $cur_class = '';
my $cur_member = '';
my $cur_type = '';

sub mkdirs
{
    my $dir = shift;
    my @dirs = (split(/\//, $dir));
    my $cur = 'gen';

    foreach my $new_dir (split(/\//, $dir)) {
	mkdir $cur;
	$cur .= "/$new_dir";
    }
}

our %P62JNI = (
    'void'  => 'V',
    'bool'  => 'Z',
    'uint8' => 'B',
    'int8'  => 'C',
    'int16' => 'S',
    'int'   => 'I',
    'int64' => 'J',
    'num32' => 'F',
    'num64' => 'D',
);

our %JNI2P6 = map { $P62JNI{$_} => $_ } keys %P62JNI;

sub objjni2objp6
{
    my $objj = shift;

    return 'Str' if $objj eq 'Ljava/lang/String;';

    $objj =~ s/^L//;
    $objj =~ s/;$//;

    my $ret = "RakuDroidRole/$objj";
    $ret =~ s,/,::,g;
    $ret =~ s/\$/__/g;

    return $ret;
}

sub objp62cljni
{
    my $obj = shift;

    return 'java.lang.String' if $obj eq 'Str';
    return $obj unless $obj =~ /^RakuDroidRole::/;

    $obj =~ s/__/\$/g;
    $obj =~ s,::,.,g;
    $obj =~ s,^RakuDroidRole\.,,;

    return $obj;
}

sub sigjni2uses
{
    my $sig = shift;

    my %seen;
    $seen{objjni2objp6($1)}++ while $sig =~ /(L[^;]+;)/og;
    delete $seen{Str};

    return keys %seen;
}

sub sigjni2sigp6
{
    my $sig = shift;

    my (@objs, @sigobjs, $arr_level, $ret_obj, $seen_paren);
    $arr_level = $seen_paren = 0;

    push @objs, $1 while $sig =~ s/(L[^;]+;)/O/o;

    foreach my $c (split('', $sig)) {
	my $obj6;
	if ($c eq 'O') {
	    $obj6 = objjni2objp6(shift @objs);
	} elsif ($c eq '[') {
	    $arr_level++;
	    next;
	} elsif ($c =~ /[\(\)]/) {
	    $seen_paren++;
	    $ret_obj++ if $c eq ')';
	    next;
	} else {
	    $obj6 = $JNI2P6{$c};
	}

	$obj6 = ('CArray[' x $arr_level) . $obj6 . (']' x $arr_level);
	$arr_level = 0;
	push @sigobjs, $obj6;
    }

    if ($ret_obj || !$seen_paren) {
	$ret_obj = pop(@sigobjs);
    } else {
	$ret_obj = '';
    }

    my $argnb = 1;
    return (join(', ', map { "$_ \$arg" . $argnb++ } @sigobjs), $ret_obj, 0+@sigobjs);
}

my ($protec_all, $protec, $final, $abstract, $type, $name, $params, $extends_all, $extends, $extends_params, $impl_all, $impl, $static, $throws_all, $throws, $sig, $item_h);
my %seen_uses;

# /data/data/com.example.myapplication/files/rakudroid/lib/RakuDroid/android/security/keystore/KeyProperties.pm6
# /data/data/com.example.myapplication/files/rakudroid/lib/RakuDroid/android/view/Menu.pm6
# /data/data/com.example.myapplication/files/rakudroid/lib/RakuDroid/com/android/internal/util/Predicate.pm6

foreach my $line (<>) {
    1 while $line =~ s/\<[^\<\>]+\>//g;
#    if (($protec_all, $protec, $final, $abstract, $type, $name, $params, $extends_all, $extends, $extends_params, $impl_all, $impl) = $line =~ /^((public|protected) )?(final )?(abstract )?(class|interface) ([\w\.\$]+)(\<.+)? (extends([, ]+([\w\.\$]+)(\<.+)?)+)?\s*(implements([, ]+([\w\.\$]+)(\<.+)?)+)?\s*\{/) {
    if (($protec_all, $protec, $final, $abstract, $type, $name, $extends_all, $extends, $impl_all, $impl) = $line =~ /^((public|protected) )?(final )?(abstract )?(class|interface) ([\w\.\$]+) (extends([, ]+([\w\.\$]+))+)?\s*(implements([, ]+([\w\.\$]+))+)?\s*\{/) {

	$type = 'role' if $type eq 'interface';
	# if (defined($impl)) {
	#     $impl =~ s/\<[^>]+\>//g;
	# }

	$classes{$name} = {
	    type     => $type,
	    protec   => $protec,
	    final    => $final,
	    abstract => $abstract,
	    name     => $name,
	    extends  => $extends,
	    impl     => defined($impl) ? [ split(/\s*,\s*/, $impl) ] : undef,
	    methods  => {},
	    fields   => {},
	};

	$classes{$cur_class}{uses} = [ keys %seen_uses ] if $cur_class ne '';
	$cur_class = $name;
	%seen_uses = ();

    } elsif (($sig) = $line =~ /^    descriptor: ([\w\/;\$\(\)]+)/) {
	$item_h->{sig} = $sig;
	$seen_uses{$_}++ for sigjni2uses($sig);
	if ($cur_type eq 'methods') {
	    push @{$classes{$cur_class}{$cur_type}{$cur_member}}, $item_h;
	} else {
	    $classes{$cur_class}{$cur_type}{$cur_member} = $item_h;
	}
    } elsif (($protec_all, $protec, $static, $final, $abstract, $name, $throws_all, $throws) = $line =~ /^  ((public|protected) )?(static )?(final )?(abstract )?.*?([\w\.\$]+)\(.*\)( throws (\S+))?;/) {
	$classes{$cur_class}{methods}{$name} //= [];
	$name = 'new' if $name eq $cur_class;

	$item_h = {
	    protec   => $protec,
	    static   => $static,
	    final    => $final,
	    abstract => $abstract,
	    name     => $name,
	};
	$cur_type = 'methods';
	$cur_member = $name;
    } elsif (($protec_all, $protec, $static, $final, $name) = $line =~ /^  ((public|protected) )?(static )?(final )?.*?([\w\.\$]+);/) {
	$name =~ s/\./-/g;
	$item_h = {
	    protec   => $protec,
	    static   => $static,
	    final    => $final,
	    name     => $name,
	};
	$cur_type = 'fields';
	$cur_member = $name;
    }
}

foreach my $class (keys %classes) {
    my $n_class = "RakuDroid.$class";
    $n_class =~ s/\$/__/g;
    my $path = $n_class;
    $path =~ s,\.,/,g;
    $path =~ s/$.*//;
    mkdirs($path);
    my $role_path = $path;
    $role_path =~ s,/,Role/,;
    mkdirs($role_path);

    $n_class =~ s/\./::/g;
    my $role = $n_class;
    $role =~ s/::/Role::/;

    open(OUTROLE, '>', "gen/$role_path.pm6") or die $!;
    say OUTROLE "# GENERATED, don't edit or you'll loose!
role $role {}";
    close(OUTROLE);

    open(OUT, '>', "gen/$path.pm6") or die $!;

    say OUT "# GENERATED, don't edit or you'll loose!

use $role;
unit $classes{$class}{type} $n_class does $role;

use RakuDroid;
use NativeCall :types;
";

    foreach my $used (sort grep { $_ ne $n_class } @{$classes{$class}{uses}}) {
	my $usedjni = objp62cljni($used);
#	say OUT "try require $used;" if exists($classes{$usedjni}) && $classes{$usedjni}{type} ne 'role';
	say OUT "use $used;" if exists($classes{$usedjni});
    }

    say OUT "
my RakuDroid \$rd = RakuDroid.new(:class-name('$class'));

";

    foreach my $method (keys %{$classes{$class}{methods}}) {
	foreach my $method_h (@{$classes{$class}{methods}{$method}}) {
	    next unless grep { exists($classes{objp62cljni($_)}) } sigjni2uses($method_h->{sig});
	    my ($args, $ret, $nbargs) = sigjni2sigp6($method_h->{sig});
	    my $sigp6 = $args;
	    $sigp6 .= " --> $ret" if $ret && $ret ne 'void';
	    say OUT "multi method $method_h->{name}($sigp6)
{
    return \$rd.method-invoke(self, '$class', '$method_h->{name}', '$method_h->{sig}'" . join('', map { ", \$arg$_" } 1..$nbargs) . ");
}
";
	}
    }

    foreach my $field_h (values %{$classes{$class}{fields}}) {
	next unless grep { exists($classes{objp62cljni($_)}) } sigjni2uses($field_h->{sig});
	$field_h->{name} =~ s/\$/__/g;
	my ($args, $ret, $nbargs) = sigjni2sigp6($field_h->{sig});
	say OUT "has \$.$field_h->{name};
method $field_h->{name}() is rw {
    my bool \$cached = False;
    return Proxy.new:
	FETCH => sub (\$) {
	    \$!$field_h->{name} = \$rd.field-get('$field_h->{name}', '$field_h->{sig}') unless \$cached;
	    \$cached = True;
	    return \$!$field_h->{name};
	},
	STORE => sub (\$, \$$field_h->{name}) {
	    \$cached = True;
	    \$!$field_h->{name} = \$$field_h->{name};
	}
}
";
    }

    close(OUT);
}
