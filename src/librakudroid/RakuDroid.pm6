unit module RakuDroid;

use NativeCall;
use MONKEY-SEE-NO-EVAL;

sub rakudo_p6_init(& (Str --> Str)) is native('rakudroid') { * }
sub rakudo_p6_set_ok(int64) is native('rakudroid') { * }

rakudo_p6_init(sub (Str $code --> Str(Any)) {
		      module RakuDroidRun {
			  my $ret = EVAL $code;

			  CATCH {
			      default {
				  rakudo_p6_set_ok(0);
				  return .message;
			      }
			  }

			  CONTROL {
			      when CX::Warn {
				  .message.note;
				  .resume;
			      }
			  }

			  rakudo_p6_set_ok(1);
			  return $ret;
		      }
		  });
