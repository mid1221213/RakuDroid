use NativeCall :types;

class RakuDroidJValueUnion is repr('CUnion')
{
    has Str     $.str;
    has uint8   $.bool;
    has uint8   $.uint8;
    has int8    $.int8;
    has int16   $.int16;
    has int64   $.int;
    has int64   $.int64;
    has num32   $.num32;
    has num64   $.num64;
    has Pointer $.pointer;

    submethod BUILD(:$type, :$val) {
	given $type {
	    when 's' { $!str     := $val.Str }
	    when 'Z' { $!bool     = $val.Int }
	    when 'B' { $!uint8    = $val.Int }
	    when 'C' { $!int8     = $val.Int }
	    when 'S' { $!int16    = $val.Int }
	    when 'I' { $!int      = $val.Int }
	    when 'J' { $!int64    = $val.Int }
	    when 'F' { $!num32    = $val.Num }
	    when 'D' { $!num64    = $val.Num }
	    default  { $!pointer := Pointer.new($val.Int) }
	}
    }
}

class RakuDroidJValue is repr('CStruct')
{
    has uint8 $.type;
    has RakuDroidJValueUnion $!val;

    submethod BUILD(Str :$type, :$val) {
	$!type = $type.ord;
	$!val := RakuDroidJValueUnion.new(:type($type), :val($val));
    }

    method gist() {
	given $!type {
	    when ord('s') { $!val.str.Str     }
	    when ord('Z') { $!val.bool.so.Str }
	    when ord('B') { my uint8 $val = $!val.uint8; $val.Str }
	    when ord('C') { $!val.int8.Str    }
	    when ord('S') { $!val.int16.Str   }
	    when ord('I') { $!val.int.Str     }
	    when ord('J') { $!val.int64.Str   }
	    when ord('F') { $!val.num32.Str   }
	    when ord('D') { $!val.num64.Str   }
	    default  { 'Pointer.new(0x' ~ $!val.pointer.Int.base(16) ~ ')' }
	}
    }

    method Str() { self.gist }

    method Int() {
	given $!type {
	    when ord('s') { $!val.str.Int     }
	    when ord('Z') { $!val.bool.so.Int }
	    when ord('B') { my uint8 $val = $!val.uint8; $val.Int }
	    when ord('C') { $!val.int8.Int    }
	    when ord('S') { $!val.int16.Int   }
	    when ord('I') { $!val.int.Int     }
	    when ord('J') { $!val.int64.Int   }
	    when ord('F') { $!val.num32.Int   }
	    when ord('D') { $!val.num64.Int   }
	    default  { $!val.pointer.Int }
	}
    }

    method Num() {
	given $!type {
	    when ord('s') { $!val.str.Num     }
	    when ord('Z') { $!val.bool.so.Num }
	    when ord('B') { my uint8 $val = $!val.uint8; $val.Num }
	    when ord('C') { $!val.int8.Num    }
	    when ord('S') { $!val.int16.Num   }
	    when ord('I') { $!val.int.Num     }
	    when ord('J') { $!val.int64.Num   }
	    when ord('F') { $!val.num32.Num   }
	    when ord('D') { $!val.num64.Num   }
	    default  { $!val.pointer.Num }
	}
    }

    method so() { self.Int.so }
}
