unit class RakuDroid;

use NativeCall::Types;
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
	given $p.type {
	    when bool  { @a.push(RakuDroidJValue.new(:bool(@args.shift))) }
	    when uint8 { @a.push(RakuDroidJValue.new(:uint8(@args.shift))) }
	    when int8  { @a.push(RakuDroidJValue.new(:int8(@args.shift))) }
	    when int16 { @a.push(RakuDroidJValue.new(:int16(@args.shift))) }
	    when int   { @a.push(RakuDroidJValue.new(:int(@args.shift))) }
	    when int64 { @a.push(RakuDroidJValue.new(:int64(@args.shift))) }
	    when num32 { @a.push(RakuDroidJValue.new(:num32(@args.shift))) }
	    when num64 { @a.push(RakuDroidJValue.new(:num64(@args.shift))) }
	    default    { @a.push(RakuDroidJValue.new(:pointer(@args.shift.j-obj))) }
	}
    }

    return @a;
}

method method-invoke($obj, Str $name, Str $sig, Signature $s, *@args)
{
    return RakuDroidHelper::method-invoke(self, $obj, $name, $sig, process-args(@args));
}

method static-method-invoke(Str $name, Str $sig, Signature $s, *@args)
{
    return RakuDroidHelper::static-method-invoke(self, $name, $sig, process-args(@args));
}
