use NativeCall :types;

class RakuDroidJValue is repr('CStruct')
{
    class JUnion is repr('CUnion')
    {
	has Str     $.str;
	has uint8   $.bool;
	has int8    $.byte;
	has uint16  $.char;
	has int16   $.short;
	has int32   $.int;
	has int64   $.long;
	has num32   $.float;
	has num64   $.double;
	has Pointer $.object;

	submethod BUILD(:$type, :$val) {
	    given $type {
		when 's' { $!str    := $val.Str }
		when 'Z' { $!bool    = $val.Int }
		when 'B' { $!byte    = $val.Int }
		when 'C' { $!char    = $val.Int }
		when 'S' { $!short   = $val.Int }
		when 'I' { $!int     = $val.Int }
		when 'J' { $!long    = $val.Int }
		when 'F' { $!float   = $val.Num }
		when 'D' { $!double  = $val.Num }
		default  { $!object := Pointer.new($val.Int) }
	    }
	}
    }

    has uint8 $.type;
    has JUnion $!val;

    submethod BUILD(Str :$type, :$val) {
	$!type = $type.ord;
	$!val := JUnion.new(:type($type), :val($val));
    }

    method gist() {
	given $!type {
	    when 's'.ord { $!val.str.Str     }
	    when 'Z'.ord { $!val.bool.so.Str }
	    when 'B'.ord { $!val.byte.Str    }
	    when 'C'.ord { my uint16 $val = $!val.char; $val.Str }
	    when 'S'.ord { $!val.short.Str   }
	    when 'I'.ord { $!val.int.Str     }
	    when 'J'.ord { $!val.long.Str    }
	    when 'F'.ord { $!val.float.Str   }
	    when 'D'.ord { $!val.double.Str  }
	    default  { 'Pointer.new(0x' ~ $!val.object.Int.base(16) ~ ')' }
	}
    }

    method Str() { self.gist }

    method Int() {
	given $!type {
	    when 's'.ord { $!val.str.Int     }
	    when 'Z'.ord { $!val.bool.so.Int }
	    when 'B'.ord { $!val.byte.Int    }
	    when 'C'.ord { my uint16 $val = $!val.char; $val.Int }
	    when 'S'.ord { $!val.short.Int   }
	    when 'I'.ord { $!val.int.Int     }
	    when 'J'.ord { $!val.long.Int    }
	    when 'F'.ord { $!val.float.Int   }
	    when 'D'.ord { $!val.double.Int  }
	    default  { $!val.object.Int }
	}
    }

    method so() { self.Int.so }

    method Num() {
	given $!type {
	    when 's'.ord { $!val.str.Num     }
	    when 'Z'.ord { $!val.bool.so.Num }
	    when 'B'.ord { $!val.byte.Num    }
	    when 'C'.ord { my uint16 $val = $!val.char; $val.Num }
	    when 'S'.ord { $!val.short.Num   }
	    when 'I'.ord { $!val.int.Num     }
	    when 'J'.ord { $!val.long.Num    }
	    when 'F'.ord { $!val.float.Num   }
	    when 'D'.ord { $!val.double.Num  }
	    default  { $!val.object.Num }
	}
    }
}
