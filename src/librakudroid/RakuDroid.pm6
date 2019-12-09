unit class RakuDroid;

use NativeCall :types;
use RakuDroidJValue;

# has CPointer $!JObject;
my %classes;
has Str $.class-name;

##########################################
# use MONKEY-TYPING;
# use RakuDroidRole::java::lang::String;
# augment class Str {
#     also does RakuDroidRole::java::lang::String;
# }
# augment class Int {
#     method int8()   { my int8   $x = self; $x }
#     method uint8()  { my uint8  $x = self; $x }
#     method int16()  { my int16  $x = self; $x }
#     method uint16() { my uint16 $x = self; $x }
#     method int32()  { my int32  $x = self; $x }
#     method uint32() { my uint32 $x = self; $x }
#     method int64()  { my int64  $x = self; $x }
#     method uint64() { my uint64 $x = self; $x }
# }
##########################################

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
	$p.type.note;
	$*ERR.flush;
	given $p.type {
	    when Str    { @a.push(RakuDroidJValue.new(:type<s>, :val($ret))) }
	    when bool   { @a.push(RakuDroidJValue.new(:type<Z>, :val($ret))) }
	    when int8   { @a.push(RakuDroidJValue.new(:type<B>, :val($ret))) }
	    when uint16 { @a.push(RakuDroidJValue.new(:type<C>, :val($ret))) }
	    when int16  { @a.push(RakuDroidJValue.new(:type<S>, :val($ret))) }
	    when int32  { @a.push(RakuDroidJValue.new(:type<I>, :val($ret))) }
	    when int64  { @a.push(RakuDroidJValue.new(:type<J>, :val($ret))) }
	    when num32  { @a.push(RakuDroidJValue.new(:type<F>, :val($ret))) }
	    when num64  { @a.push(RakuDroidJValue.new(:type<D>, :val($ret))) }
	    default     {
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

method ctor-invoke(Str $sig, Signature $p6sig, *@args)
{
    return RakuDroidHelper::ctor-invoke(self, $sig, process-args($p6sig, @args));
}

method method-invoke($obj, Str $name, Str $sig, Signature $p6sig, *@args)
{
    return RakuDroidHelper::method-invoke(self, $obj, $name, $sig, process-args($p6sig, @args));
}

method static-method-invoke(Str $name, Str $sig, Signature $p6sig, *@args)
{
    return RakuDroidHelper::static-method-invoke(self, $name, $sig, process-args($p6sig, @args));
}
