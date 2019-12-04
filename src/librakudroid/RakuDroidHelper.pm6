unit class RakuDroidHelper;

use NativeCall;
use RakuDroidJValue;

use MONKEY-TYPING;
use RakuDroidRole::java::lang::String;
augment class Str {
    also does RakuDroidRole::java::lang::String;
}

sub rakudo_p6_init(& (Str --> Str), & (Pointer --> Str)) is native('rakudroid') { * }
sub rakudo_p6_set_ok(int64) is native('rakudroid') { * }

sub ctor_invoke(         Str,               Str, CArray[RakuDroidJValue]      --> Pointer                 ) is native { * }
sub method_invoke(       Str, Pointer, Str, Str, CArray[RakuDroidJValue], Str --> Pointer[RakuDroidJValue]) is native { * }
sub static_method_invoke(Str,          Str, Str, CArray[RakuDroidJValue], Str --> Pointer[RakuDroidJValue]) is native { * }

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
    require RakuDroid::android::app::Activity;
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

sub common-invoke-pre(Str $sig --> Str)
{
    my $ret-type = $sig.chars ?? substr($sig, *) !! 'V';

    if $ret-type eq ';' {
	$ret-type = $sig;
	$ret-type ~~ s/L (<-[\;]>+) \; $ /$0/;

	return 's' if $ret-type eq 'java/lang/string'; # special Str case

	$ret-type ~~ s:g,/,::,;
	$ret-type ~~ s:g/\$/__/;
	$ret-type ~~ s/^/RakuDroid::/;

	return $ret-type;
    }

    return $ret-type;
}

sub common-invoke-post(Str $ret-type, RakuDroidJValue $ret)
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
	    if ::($ret-type) ~~ Failure {
		require ::($ret-type);
	    }

	    return ::($ret-type).bless(j-obj => $ret.val.pointer);
	}
    }
}

our sub ctor-invoke($rd, $sig, @args)
{
    my $c-args := CArray[RakuDroidJValue].new(@args);
    $c-args[@args.elems] = 0;

    my $ret-type = $rd.class-name;
    $ret-type ~~ s:g,/,::,;
    $ret-type ~~ s:g/\$/__/;
    $ret-type ~~ s/^/RakuDroid::/;

    if ::($ret-type) ~~ Failure {
	require ::($ret-type);
    }

    return ::($ret-type).bless(j-obj => ctor_invoke($rd.class-name, $sig, $c-args));
}

our sub method-invoke($rd, $obj, $name, $sig, @args)
{
    my $c-args := CArray[RakuDroidJValue].new(@args);
    $c-args[@args.elems] = 0;

    my $ret-type = common-invoke-pre($sig);

    my RakuDroidJValue $ret = method_invoke($rd.class-name, $obj.j-obj, $name, $sig, $c-args, $ret-type);

    return common-invoke-post($ret-type, $ret);
}

our sub static-method-invoke($rd, $name, $sig, @args)
{
    my $c-args := CArray[RakuDroidJValue].new(@args);
    $c-args[@args.elems] = 0;

    my $ret-type = common-invoke-pre($sig);

    my RakuDroidJValue $ret = static_method_invoke($rd.class-name, $name, $sig, $c-args, $ret-type);

    return common-invoke-post($ret-type, $ret);
}
