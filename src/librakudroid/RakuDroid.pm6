unit class RakuDroid;

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

method method-invoke($obj, Str $name, Str $sig, *@args)
{
    return RakuDroidHelper::method-invoke(self, $obj, $name, $sig, @args);
}
