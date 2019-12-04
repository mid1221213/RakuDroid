unit class RakuDroid;

use NativeCall :types;
use RakuDroidJValue;

# has CPointer $!JObject;
my %classes;
has Str $.class-name;

method new-obj()
{
    return $!class-name; # TBD
}

method TWEAK()
{
    note "TWEAKing class $!class-name";
    unless %classes{$!class-name}:exists {
	%classes{$!class-name} = self.new-obj;
	note "created obj of class $!class-name";
    }
}

method field-get($name, $type)
{
#    return field_get($name, $type);
}

sub process-args(Signature $s, @args --> Array[RakuDroidJValue])
{
    my RakuDroidJValue @a;

    for $s.params -> $p {
	my $ret = @args.shift;
	given $p.type {
	    when Str   { @a.push(RakuDroidJValue.new-with-val('s', $ret)) }
	    when bool  { @a.push(RakuDroidJValue.new-with-val('Z', $ret)) }
	    when uint8 { @a.push(RakuDroidJValue.new-with-val('B', $ret)) }
	    when int8  { @a.push(RakuDroidJValue.new-with-val('C', $ret)) }
	    when int16 { @a.push(RakuDroidJValue.new-with-val('S', $ret)) }
	    when int   { @a.push(RakuDroidJValue.new-with-val('I', $ret)) }
	    when int64 { @a.push(RakuDroidJValue.new-with-val('J', $ret)) }
	    when num32 { @a.push(RakuDroidJValue.new-with-val('F', $ret)) }
	    when num64 { @a.push(RakuDroidJValue.new-with-val('D', $ret)) }
	    default    {
		@a.push(RakuDroidJValue.new-with-val('s', $ret)) if $ret ~~ Str;
		@a.push(RakuDroidJValue.new-with-val(';', $ret.j-obj));
	    }
	}
    }

    return @a;
}

method ctor-invoke(Str $sig, Signature $s, *@args)
{
    return RakuDroidHelper::ctor-invoke(self, $sig, process-args($s, @args));
}

method method-invoke($obj, Str $name, Str $sig, Signature $s, *@args)
{
    return RakuDroidHelper::method-invoke(self, $obj, $name, $sig, process-args($s, @args));
}

method static-method-invoke(Str $name, Str $sig, Signature $s, *@args)
{
    return RakuDroidHelper::static-method-invoke(self, $name, $sig, process-args($s, @args));
}
