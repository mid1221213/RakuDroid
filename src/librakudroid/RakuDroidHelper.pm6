unit class RakuDroidHelper;

use NativeCall;

use MONKEY-SEE-NO-EVAL;

sub rakudo_p6_init(& (Str --> Str)) is native('rakudroid') { * }
sub rakudo_p6_set_ok(int64) is native('rakudroid') { * }
our sub method_invoke(Str, Str, CArray[Pointer], int64) is native('rakudroid') { * }

rakudo_p6_init(sub (Str $code --> Str(Any)) {
		      my $ret = EVAL $code;

		      CATCH {
			  default {
			      rakudo_p6_set_ok(0);
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
		  });

our sub method-invoke($name, $sig, @args)
{
    my @c-args := CArray[Pointer].new(@args);

    return method_invoke($name, $sig, @c-args, @c-args.elems);
}
