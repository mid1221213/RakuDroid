unit class RakuDroidHelper;

use NativeCall;

use MONKEY-SEE-NO-EVAL;

sub rakudo_p6_init(& (Str --> Str), & (Pointer --> Str)) is native('rakudroid') { * }
sub rakudo_p6_set_ok(int64) is native('rakudroid') { * }
sub method_invoke(Str, Str, Str, CArray[Pointer], uint32) is native('rakudroid') { * }

sub helper_eval(Str $code --> Str(Any))
{
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
    $main-activity = RakuDroid::android::app::Activity.CREATE;
    $main-activity.j-obj = $ptr;

    rakudo_p6_set_ok(1);
    return 'OK';

    CATCH {
	default {
	    return .perl;
	}
    }
}

rakudo_p6_init(&helper_eval, &helper_init_activity);

our sub method-invoke($rd, $obj, $name, $sig, @args)
{
    my @c-args := CArray[Pointer].new(@args);

    return method_invoke($rd.class-name, $name, $sig, @c-args, @c-args.elems);
}
