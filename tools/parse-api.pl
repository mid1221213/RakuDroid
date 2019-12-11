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
    'void'   => 'V',
    'bool'   => 'Z',
    'int8'   => 'B',
    'uint16' => 'C',
    'int16'  => 'S',
    'int32'  => 'I',
    'int64'  => 'J',
    'num32'  => 'F',
    'num64'  => 'D',
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

    return 'java/lang/String' if $obj eq 'Str';
    return $obj unless $obj =~ /^RakuDroidRole::/;

    $obj =~ s/__/\$/g;
    $obj =~ s,::,/,g;
    $obj =~ s,^RakuDroidRole/,,;

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

    my (@objs, @sigobjs, @sigobjs_coercion, $arr_level, $ret_obj, $seen_paren, $seen_ret);
    $arr_level = $seen_paren = $seen_ret = 0;

    push @objs, $1 while $sig =~ s/(L[^;]+;)/O/o;

    foreach my $c (split('', $sig)) {
	my ($obj6, $obj6_coercion) = ("") x 2;
	$seen_ret++ if $ret_obj;
	if ($c eq 'O') {
	    $obj6 = objjni2objp6(shift @objs);
	} elsif ($c eq '[') {
	    $arr_level++;
	    next;
	} elsif ($c =~ /[\(\)]/) {
	    $seen_paren++;
	    if ($c eq ')') {
		$ret_obj++;
		$seen_ret = 0;
	    }
	    next;
	} else {
	    $obj6 = $JNI2P6{$c};
	    if (!$arr_level && $c =~ /^[BCSIJ]$/) {
		$obj6_coercion = "$obj6(Int)";
	    }
	}

	$obj6 = ('Array[' x $arr_level) . $obj6 . (']' x $arr_level);
	$arr_level = 0;
	push @sigobjs, $obj6;
	if ($obj6_coercion) {
	    push @sigobjs_coercion, $obj6_coercion;
	} else {
	    push @sigobjs_coercion, $obj6;
	}
    }

    if ($seen_ret || !$seen_paren) {
	$ret_obj = pop(@sigobjs);
	pop(@sigobjs_coercion);
    } else {
	$ret_obj = '';
    }

    my $argnb = 1;
    return (join(', ', map { "$_ \$arg" . $argnb++ } @sigobjs_coercion), join(', ', @sigobjs), $ret_obj, 0+@sigobjs);
}

my $item_h;
my %seen_uses;
my %seen_ruses;

my $regex = qr/^
    ((
      public|
      protected
    )
    \s)?
    (final\s)?
    (abstract\s)?
    (class|interface)
    \s
    ([\w\.\$]+)
#    (\<.+\>)?
    \s
    (
      extends\s
      (
        ([\w\.\$]+)
#        (\<.+\>)?
        (
          \s*,\s*
          ([\w\.\$]+)
#          (\<.+\>)?
        )*
      )?\s
    )?
    (
      implements\s
      (
        ([\w\.\$]+)
#        (\<.+\>)?
        (
          \s*,\s*
          ([\w\.\$]+)
#          (\<.+\>)?
        )*
      )?\s
    )?
    \s*\{
/x;

my $dum;
my ($protec_all, $protec, $final, $abstract, $type, $name, $extends_all, $extends, $impl_all, $impl);
my $sig;
my ($static, $throws_all, $throws);

foreach my $line (<>) {
    1 while $line =~ s/\<[^\<\>]+\>//g;
    if (($protec_all, $protec, $final, $abstract, $type, $name, $extends_all, $extends, $dum, $dum, $dum, $impl_all, $impl) = $line =~ /^$regex/) {

	$type = 'role' if $type eq 'interface';
	if (defined($extends)) {
	    $extends =~ s/^\s+//;
	    $extends =~ s,\.,/,g;
	}
	if (defined($impl)) {
	    $impl =~ s/^\s+//;
	    $impl =~ s,\.,/,g;
	}

	$name =~ s,\.,/,g;
	$classes{$name} = {
	    type     => $type,
	    protec   => $protec // 'public',
	    final    => $final,
	    abstract => $abstract,
	    name     => $name,
	    extends  => defined($extends) ? [ split(/\s*,\s*/, $extends) ] : [],
	    impl     => defined($impl) ? [ split(/\s*,\s*/, $impl) ] : [],
	    methods  => {},
	    fields   => {},
	};

	$classes{$cur_class}{uses}  = [ keys %seen_uses  ] if $cur_class ne '';
	$classes{$cur_class}{ruses} = [ keys %seen_ruses ] if $cur_class ne '';

	$cur_class = $name;

	%seen_uses  = ();
	%seen_ruses = ();

	if (defined($extends)) {
	    $seen_uses{objjni2objp6($_)}++ for split(/\s*,\s*/, $extends);
	    $seen_ruses{objjni2objp6($_)}++ for split(/\s*,\s*/, $extends);
	}
	if (defined($impl)) {
	    $seen_uses{objjni2objp6($_)}++ for split(/\s*,\s*/, $impl);
	    $seen_ruses{objjni2objp6($_)}++ for split(/\s*,\s*/, $impl);
	}

    } elsif (($sig) = $line =~ /^    descriptor: ([\w\/;\$\(\)\[]+)/) {
	$item_h->{sig} = $sig;
	$seen_uses{$_}++ for sigjni2uses($sig);
	if ($cur_type eq 'methods') {
	    push @{$classes{$cur_class}{$cur_type}{$cur_member}}, $item_h;
	} else {
	    $classes{$cur_class}{$cur_type}{$cur_member} = $item_h;
	}
    } elsif (($protec, $static, $final, $abstract, $name, $throws_all, $throws) = $line =~ /^  (public|protected) (static )?(final )?(abstract )?.*?([\w\.\$]+)\(.*\)( throws (\S+))?;/) {
	$classes{$cur_class}{methods}{$name} //= [];

	$name =~ s,\.,/,g;
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
    } elsif (($dum, $protec, $static, $final, $name) = $line =~ /^  ((public|protected) )?(static )?(final )?.*?([\w\.\$]+);/) {
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

$classes{$cur_class}{uses}  = [ keys %seen_uses ]  if $cur_class ne '';
$classes{$cur_class}{ruses} = [ keys %seen_ruses ] if $cur_class ne '';

mkdirs('gen/provides');
open(OUTPROVS, '>', "gen/provides") or die $!;
say OUTPROVS "RakuDroid src/librakudroid/RakuDroid.pm6";
say OUTPROVS "RakuDroidJValue src/librakudroid/RakuDroidJValue.pm6";
say OUTPROVS "RakuDroidRole gen/RakuDroidRole.pm6";

my %role_deps = (
    text => {},
    deps => {},
    done => {},
);

foreach my $class (keys %classes) {
    my $n_class = "RakuDroid/$class";
    $n_class =~ s/\$/__/g;
    my $path = $n_class;
    mkdirs($path);

    $n_class =~ s,/,::,g;
    my $role = $n_class;
    $role =~ s/::/Role::/;

    say OUTPROVS "$n_class gen/$path.pm6";

    $role_deps{text}{$role} = "role $role {
";

    open(OUT, '>', "gen/$path.pm6") or die $!;
    say OUT "# GENERATED, don't edit or you'll loose!

use RakuDroidRole;
";

    foreach my $used (sort grep { $_ ne $n_class } @{$classes{$class}{uses}}) {
	my $usedjni = objp62cljni($used);
	say OUT "use $used;" if exists($classes{$usedjni}) && $used ne $role;
    }

    my $is_str = '';
    $is_str = ' is Str' if $n_class eq 'RakuDroid::java::lang::String';

    say OUT "
unit $classes{$class}{type} $n_class$is_str does $role;
";

    $role_deps{text}{$role} .= "
multi method new(Str \$str)
{
    self.bless(:value(\$str))
}
" if $n_class eq 'RakuDroid::java::lang::String';

    foreach my $extends (sort @{$classes{$class}{extends}}) {
	my $extendsp6 = objjni2objp6($extends);
	$role_deps{text}{$role} .= "also does $extendsp6;
";
	$role_deps{deps}{$role}{$extendsp6}++;
	say OUT "also does $extendsp6;";
    }

    foreach my $impl (sort @{$classes{$class}{impl}}) {
	my $implp6 = objjni2objp6($impl);
	$role_deps{text}{$role} .= "also does $implp6;
";
	$role_deps{deps}{$role}{$implp6}++;
	say OUT "also does $implp6;";
    }

    say OUT "
use RakuDroid;
use NativeCall :types;

my RakuDroid \$rd = RakuDroid.new(:class-name('$class'));
has Pointer \$.j-obj is rw;
";

    foreach my $method (sort keys %{$classes{$class}{methods}}) {
	my $multi = @{$classes{$class}{methods}{$method}} > 1 ? 'multi ' : '';
	say OUT "our proto $method(|) { * }" if $multi && defined($classes{$class}{methods}{$method}[0]{static});

	foreach my $method_h (@{$classes{$class}{methods}{$method}}) {
	    my @sig_uses = sigjni2uses($method_h->{sig});
	    next unless @sig_uses == 0 || @sig_uses == grep { exists($classes{objp62cljni($_)}) } @sig_uses;
	    my ($args, $args_s, $ret, $nbargs) = sigjni2sigp6($method_h->{sig});
	    my $sigp6 = $args;
	    $sigp6 .= " --> $ret" if $ret && $ret ne 'void';
	    $sigp6 =~ s/^\s+//;

	    if ($method_h->{name} eq 'new') {
		say OUT "${multi}method $method_h->{name}($sigp6)
{
    return \$rd.ctor-invoke('$method_h->{sig}', :($args_s)" . join('', map { ", \$arg$_" } 1..$nbargs) . ");
}
";
	    } elsif (defined($method_h->{static})) {
		say OUT "${multi}sub $method_h->{name}($sigp6)
{
    return \$rd.static-method-invoke('$method_h->{name}', '$method_h->{sig}', :($args_s)" . join('', map { ", \$arg$_" } 1..$nbargs) . ");
}
";
	    } else {
	    say OUT "${multi}method $method_h->{name}($sigp6)
{
    return \$rd.method-invoke(self, '$method_h->{name}', '$method_h->{sig}', :($args_s)" . join('', map { ", \$arg$_" } 1..$nbargs) . ");
}
"
	    };
	}
    }

    foreach my $field_h (sort { $a->{name} cmp $b->{name} } values %{$classes{$class}{fields}}) {
	my @sig_uses = sigjni2uses($field_h->{sig});
	next unless @sig_uses == 0 || @sig_uses == grep { exists($classes{objp62cljni($_)}) } @sig_uses;
	$field_h->{name} =~ s/\$/__/g;
	my $static = $field_h->{static};
	my $final = $field_h->{final};

	if ($static) {
	    say OUT "our sub $field_h->{name}(\$new-val?)
{
    if \$new-val.defined {";
	    if ($final) {
		say OUT "	die 'cannot modify <$field_h->{name}>';";
	    } else {
		say OUT "	\$rd.field-set(self, '$field_h->{name}', '$field_h->{sig}', \$new-val);
	return \$new-val;";
	    }
	    say OUT "    }

    return \$rd.static-field-get('$field_h->{name}', '$field_h->{sig}');
}
";
	    } else {
	    say OUT "method $field_h->{name}(\$new-val?)
{
    if \$new-val.defined {";
	    if ($final) {
		say OUT "	die 'cannot modify <$field_h->{name}>';";
	    } else {
		say OUT "	\$rd.field-set(self, '$field_h->{name}', '$field_h->{sig}', \$new-val);
	return \$new-val;";
	    }
	    say OUT "    }

    return \$rd.field-get(self, '$field_h->{name}', '$field_h->{sig}');
}
"
	    };
    }

    $role_deps{text}{$role} .= "}";
    close(OUT);
}

open(OUTROLE, '>', "gen/RakuDroidRole.pm6") or die $!;
    say OUTROLE "# GENERATED, don't edit or you'll loose!
";

sub out_role
{
    my ($role, $recurse_h) = @_;

    foreach my $dep (sort keys %{$role_deps{deps}{$role}}) {
	unless ($role_deps{done}{$dep}) {
	    if ($recurse_h->{$dep}) { # cannot recurse
		warn "------------------ $dep";
		$role_deps{text}{$role} = "role $dep { ... }\n" . $role_deps{text}{$role};
	    } else {
		$recurse_h->{$dep}++;
		out_role($dep, $recurse_h);
	    }
	}
    }

    if (exists($role_deps{text}{$role})) {
	say OUTROLE $role_deps{text}{$role};
    } else {
	die "$role has no text";
    }
    $role_deps{done}{$role}++;
}

foreach my $role (sort keys %{$role_deps{text}}) {
    my $recurse_h = { $role => 1 };
    out_role($role, $recurse_h) unless $role_deps{done}{$role};
}

close(OUTROLE);
close(OUTPROVS);
