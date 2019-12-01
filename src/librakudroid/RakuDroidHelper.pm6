unit class RakuDroidHelper;

use NativeCall;
use RakuDroidJValue;

use MONKEY-SEE-NO-EVAL;

sub rakudo_p6_init(& (Str --> Str), & (Pointer --> Str)) is native('rakudroid') { * }
sub rakudo_p6_set_ok(int64) is native('rakudroid') { * }

sub method_invoke(Str, Pointer, Str, Str, CArray[Pointer], uint32, Str,
		  uint8 is rw,
		  uint8 is rw,
		  int8  is rw,
		  int16 is rw,
		  int64 is rw,
		  int64 is rw,
		  num32 is rw,
		  num64 is rw
			--> Pointer) is native('rakudroid') { * }

sub static_method_invoke(Str, Str, Str, CArray[Pointer], uint32, Str,
			 uint8 is rw,
			 uint8 is rw,
			 int8  is rw,
			 int16 is rw,
			 int64 is rw,
			 int64 is rw,
			 num32 is rw,
			 num64 is rw
			       --> Pointer) is native('rakudroid') { * }

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

our sub method-invoke($rd, $obj, $name, $sig, @args)
{
    my $c-args := CArray[RakuDroidJValue].new(@args);
    $c-args[@args.elems] = 0;

    my uint8 $bool;
    my uint8 $uint8;
    my int8  $int8;
    my int16 $int16;
    my int64 $int;
    my int64 $int64;
    my num32 $num32;
    my num64 $num64;

    my $ret_type = substr($sig, *);

    my $pointer = method_invoke($rd.class-name, $obj.j-obj, $name, $sig, $c-args, @args.elems, $ret_type,
				$bool,
				$uint8,
				$int8,
				$int16,
				$int,
				$int64,
				$num32,
				$num64
			       );

    given $ret_type {
	when 'Z' { return $bool  }
	when 'B' { return $uint8 }
	when 'C' { return $int8  }
	when 'S' { return $int16 }
	when 'I' { return $int   }
	when 'J' { return $int64 }
	when 'F' { return $num32 }
	when 'D' { return $num64 }
	when ';' {
	    my $ret_ptype = $sig;
	    $ret_ptype ~~ s/L (<-[\;]>+) \; $ /$0/;
	    $ret_ptype ~~ s:g/\//\:\:/;
	    $ret_ptype ~~ s/ ^ /RakuDroid\:\:/;

	    if ::($ret_ptype) ~~ Failure {
		require ::($ret_ptype);
	    }

	    my $ret_obj = ::($ret_ptype).bless(j-obj => $pointer);
	    return $ret_obj;
	}
	default  { return }
    }
}

our sub static-method-invoke($rd, $name, $sig, @args)
{
    my $c-args := CArray[Pointer].new(@args);
    $c-args[@args.elems] = 0;

    my uint8 $bool;
    my uint8 $uint8;
    my int8  $int8;
    my int16 $int16;
    my int64 $int;
    my int64 $int64;
    my num32 $num32;
    my num64 $num64;

    my $ret_type = substr($sig, *);

    my $pointer = static_method_invoke($rd.class-name, $name, $sig, $c-args, @args.elems, $ret_type,
				       $bool,
				       $uint8,
				       $int8,
				       $int16,
				       $int,
				       $int64,
				       $num32,
				       $num64
				      );

    given $ret_type {
	when 'Z' { return $bool  }
	when 'B' { return $uint8 }
	when 'C' { return $int8  }
	when 'S' { return $int16 }
	when 'I' { return $int   }
	when 'J' { return $int64 }
	when 'F' { return $num32 }
	when 'D' { return $num64 }
	when ';' {
	    my $ret_ptype = $sig;
	    $ret_ptype ~~ s/L (<-[\;]>+) \; $ /$0/;
	    $ret_ptype ~~ s:g/\//\:\:/;
	    $ret_ptype ~~ s/ ^ /RakuDroid\:\:/;

	    if ::($ret_ptype) ~~ Failure {
		require ::($ret_ptype);
	    }

	    my $ret_obj = ::($ret_ptype).bless(j-obj => $pointer);
	    return $ret_obj;
	}
	default  { return }
    }
}
