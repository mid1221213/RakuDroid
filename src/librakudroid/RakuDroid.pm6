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

sub process-args(@args --> Array[RakuDroidJValue])
{
    my RakuDroidJValue @a;

    for Backtrace.new[4].code.signature.params -> $p {
	my $ret = @args.shift;
	given $p.type {
	    when Str   { @a.push(RakuDroidJValue.new(:type<s>, :val($ret))) }
	    when bool  { @a.push(RakuDroidJValue.new(:type<Z>, :val($ret))) }
	    when uint8 { @a.push(RakuDroidJValue.new(:type<B>, :val($ret))) }
	    when int8  { @a.push(RakuDroidJValue.new(:type<C>, :val($ret))) }
	    when int16 { @a.push(RakuDroidJValue.new(:type<S>, :val($ret))) }
	    when int   { @a.push(RakuDroidJValue.new(:type<I>, :val($ret))) }
	    when int64 { @a.push(RakuDroidJValue.new(:type<J>, :val($ret))) }
	    when num32 { @a.push(RakuDroidJValue.new(:type<F>, :val($ret))) }
	    when num64 { @a.push(RakuDroidJValue.new(:type<D>, :val($ret))) }
	    default    {
		if $ret ~~ Str {
		    @a.push(RakuDroidJValue.new(:type<s>, :val($ret)));
		} else {
		    @a.push(RakuDroidJValue.new(:type<;>, :val($ret.j-obj)));
		}
	    }
	}
    }

    return @a;
}

method ctor-invoke(Str $sig, *@args)
{
    return RakuDroidHelper::ctor-invoke(self, $sig, process-args(@args));
}

method method-invoke($obj, Str $name, Str $sig, *@args)
{
    return RakuDroidHelper::method-invoke(self, $obj, $name, $sig, process-args(@args));
}

method static-method-invoke(Str $name, Str $sig, *@args)
{
    return RakuDroidHelper::static-method-invoke(self, $name, $sig, process-args(@args));
}
