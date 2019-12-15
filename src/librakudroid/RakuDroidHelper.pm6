unit class RakuDroidHelper;

use NativeCall;
use RakuDroidJValue;
use RakuDroidRoles;

use RakuDroid::android::app::Activity;

use MONKEY-TYPING;
augment class Str {
    also does RakuDroidRole::java::lang::String;
}
augment class Int {
    method int8()   { my int8   $x = self; $x }
    method uint8()  { my uint8  $x = self; $x }
    method int16()  { my int16  $x = self; $x }
    method uint16() { my uint16 $x = self; $x }
    method int32()  { my int32  $x = self; $x }
    method uint32() { my uint32 $x = self; $x }
    method int64()  { my int64  $x = self; $x }
    method uint64() { my uint64 $x = self; $x }
}

sub rakudo_p6_init(& (Str --> Str), & (Pointer --> Str)) is native('rakudroid') { * }
sub rakudo_p6_set_ok(int64) is native('rakudroid') { * }

sub ctor_invoke(         Str,               Str, CArray[RakuDroidJValue],      RakuDroidJValue is rw --> Str) is native('rakudroid') { * }
sub method_invoke(       Str, Pointer, Str, Str, CArray[RakuDroidJValue], Str, RakuDroidJValue is rw --> Str) is native('rakudroid') { * }
sub static_method_invoke(Str,          Str, Str, CArray[RakuDroidJValue], Str, RakuDroidJValue is rw --> Str) is native('rakudroid') { * }
sub field_get(           Str, Pointer, Str, Str,                          Str, RakuDroidJValue is rw --> Str) is native('rakudroid') { * }
sub static_field_get(    Str,          Str, Str,                          Str, RakuDroidJValue is rw --> Str) is native('rakudroid') { * }
sub field_set(           Str, Pointer, Str, Str,                          Str, RakuDroidJValue       --> Str) is native('rakudroid') { * }
sub static_field_set(    Str,          Str, Str,                          Str, RakuDroidJValue       --> Str) is native('rakudroid') { * }

sub helper_eval(Str $code --> Str(Any))
{
    use MONKEY-SEE-NO-EVAL;

    my $ret = EVAL $code;

    CATCH {
	default {
	    return .message;
	}
    }

    CONTROL {
	when CX::Warn {
	    .gist.note;
	    .resume;
	}
    }

    rakudo_p6_set_ok(1);
    return $ret unless $ret === Any;
    return '<' ~ $ret.perl ~ '>';
}

our $main-activity;

sub helper_init_activity(Pointer $ptr --> Str)
{
    $main-activity = RakuDroid::android::app::Activity.bless(j-obj => $ptr);

    rakudo_p6_set_ok(1);
    return 'OK';

    CATCH {
	default {
	    return .perl;
	}
    }
}

rakudo_p6_init(&helper_eval, &helper_init_activity);

sub common-invoke-pre(Str $sig)
{
    return ('V', RakuDroidJValue.new(:type<V>, :val(0)), 'void') unless $sig.chars;

    my $real-ret-type = $sig;
    my $ret-type = substr($sig, *-1);
    my $ret;

    if $ret-type eq ';' {
	$real-ret-type ~~ s/.* L (<-[\;]>+) \; $ /$0/;
	$real-ret-type ~~ s:g/\//::/;
	$real-ret-type ~~ s:g/\$/__/;
	$real-ret-type ~~ s/^/RakuDroid::/;

	return (';', RakuDroidJValue.new(:type<s>, :val('')), $real-ret-type) if $real-ret-type eq 'RakuDroid::java::lang::String'; # special Str case

	$ret = RakuDroidJValue.new(:type<L>, :val(0));
    } elsif $ret-type eq 's' {
	$ret = RakuDroidJValue.new(:type<s>, :val(''));
    } else {
	$ret = RakuDroidJValue.new(:type($ret-type), :val(0));
    }

    return ($ret-type, $ret, $real-ret-type);
}

sub common-invoke-post(Str $ret-type, RakuDroidJValue $ret, Str $real-ret-type)
{
    given $ret-type {
	when 's' { return $ret.Str }
	when 'Z' { return $ret.so  }
	when 'B' { return $ret.Int }
	when 'C' { return $ret.Int }
	when 'S' { return $ret.Int }
	when 'I' { return $ret.Int }
	when 'J' { return $ret.Int }
	when 'F' { return $ret.Num }
	when 'D' { return $ret.Num }
	when 'V' { return }                # special no-return-value case
	default  {                         # objects
	    if ::($real-ret-type) ~~ Failure {
		require ::($real-ret-type);
	    }

	    return ::($real-ret-type).bless(j-obj => $ret.val.object);
	}
    }
}

our sub ctor-invoke($rd, Str $sig, @args)
{
    my $c-args := CArray[RakuDroidJValue].new(@args);
    $c-args[@args.elems] = RakuDroidJValue.new(:type<;>, :val(0));;

    my $ret-type = $rd.class-name;
    $ret-type ~~ s:g/\//::/;
    $ret-type ~~ s:g/\$/__/;
    $ret-type ~~ s/^/RakuDroid::/;

    if ::($ret-type) ~~ Failure {
	require ::($ret-type);
    }

    my $ret = RakuDroidJValue.new(:type<L>, :val(0));

    my $err = ctor_invoke($rd.class-name, $sig, $c-args, $ret);
    die $err if $err;

    return ::($ret-type).bless(j-obj => $ret.val.object);
}

our sub method-invoke($rd, $obj, Str $name, Str $sig, @args)
{
    my $c-args := CArray[RakuDroidJValue].new(@args);
    $c-args[@args.elems] = RakuDroidJValue.new(:type<;>, :val(0));;

    my ($ret-type, $ret, $real-ret-type) = common-invoke-pre($sig);

    my $err = method_invoke($rd.class-name, $obj.j-obj, $name, $sig, $c-args, $ret-type, $ret);
    die $err if $err;

    return common-invoke-post($ret-type, $ret, $real-ret-type);
}

our sub static-method-invoke($rd, Str $name, Str $sig, @args)
{
    my $c-args := CArray[RakuDroidJValue].new(@args);
    $c-args[@args.elems] = RakuDroidJValue.new(:type<;>, :val(0));;

    my ($ret-type, $ret, $real-ret-type) = common-invoke-pre($sig);

    my $err = static_method_invoke($rd.class-name, $name, $sig, $c-args, $ret-type, $ret);
    die $err if $err;

    return common-invoke-post($ret-type, $ret, $real-ret-type);
}

our sub field-get($rd, $obj, Str $name, Str $sig)
{
    my ($ret-type, $ret, $real-ret-type) = common-invoke-pre($sig);

    my $err = field_get($rd.class-name, $obj.j-obj, $name, $sig, $ret-type, $ret);
    die $err if $err;

    return common-invoke-post($ret-type, $ret, $real-ret-type);
}

our sub static-field-get($rd, Str $name, Str $sig)
{
    my ($ret-type, $ret, $real-ret-type) = common-invoke-pre($sig);

    my $err = static_field_get($rd.class-name, $name, $sig, $ret-type, $ret);
    die $err if $err;

    return common-invoke-post($ret-type, $ret, $real-ret-type);
}

our sub field-set($rd, $obj, Str $name, Str $sig, $val)
{
    my $err = field_set($rd.class-name, $obj.j-obj, $name, $sig, substr($sig, *-1), $val);
    die $err if $err;
}

our sub static-field-set($rd, Str $name, Str $sig, $val)
{
    my $err = static_field_set($rd.class-name, $name, $sig, substr($sig, *-1), $val);
    die $err if $err;
}
