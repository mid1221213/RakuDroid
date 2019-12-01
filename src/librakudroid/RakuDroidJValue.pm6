use NativeCall::Types;
my constant Pointer = NativeCall::Types::Pointer;

class RakuDroidJValue is repr('CUnion') {
    has uint8   $.bool;
    has uint8   $.uint8;
    has int8    $.int8;
    has int16   $.int16;
    has int64   $.int;
    has int64   $.int64;
    has num32   $.num32;
    has num64   $.num64;
    has Pointer $.pointer;
}
