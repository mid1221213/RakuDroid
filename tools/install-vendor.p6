use lib <rakudo/lib>;

my %RakuProvides = 'gen/provides'.IO.lines.split(/\s/);

class CompUnit::Repository::MyStaging is CompUnit::Repository::Installation {
    has Str $.name;

    submethod BUILD(Str :$!name --> Nil) {
        CompUnit::RepositoryRegistry.register-name($!name, self);
    }

    method short-id() { 'zeroed' }

    method name(--> Str) {
        $!name
    }
    method path-spec(CompUnit::Repository::MyStaging:D:) {
        self.^name ~ '#name(' ~ $!name ~ ')#' ~ $.prefix.absolute;
    }
    method source-file(Str $name --> IO::Path) {
        my $file = self.prefix.add($name);
        $file.e ?? $file !! self.next-repo.source-file($name)
    }
    method id() {
        return '0000000000000000000000000000000000000000';
    }
}

my %provides =
    "Test"                          => "rakudo/lib/Test.pm6",
    "NativeCall"                    => "rakudo/lib/NativeCall.pm6",
    "NativeCall::Types"             => "rakudo/lib/NativeCall/Types.pm6",
    "NativeCall::Compiler::GNU"     => "rakudo/lib/NativeCall/Compiler/GNU.pm6",
    "NativeCall::Compiler::MSVC"    => "rakudo/lib/NativeCall/Compiler/MSVC.pm6",
    "Pod::To::Text"                 => "rakudo/lib/Pod/To/Text.pm6",
    "newline"                       => "rakudo/lib/newline.pm6",
    "experimental"                  => "rakudo/lib/experimental.pm6",
    "CompUnit::Repository::Staging" => "rakudo/lib/CompUnit/Repository/Staging.pm6",
    "Telemetry"                     => "rakudo/lib/Telemetry.pm6",
    "snapper"                       => "rakudo/lib/snapper.pm6",
;

%provides<MoarVM::Profiler> = "rakudo/lib/MoarVM/Profiler.pm6" if $*VM.name eq 'moar';

PROCESS::<$REPO> := CompUnit::Repository::MyStaging.new(
    :prefix(@*ARGS[0]),
    :next-repo(
        # Make CompUnit::Repository::MyStaging available to precomp processes
        CompUnit::Repository::Installation.new(
            :prefix(@*ARGS[0]),
            :next-repo(CompUnit::RepositoryRegistry.repository-for-name('core')),
        )
    ),
    :name('vendor'),
);
$*REPO.install(
    Distribution::Hash.new(
        {
            name     => 'RAKUDROID',
            auth     => 'MIDNITE',
            ver      => $*PERL.version.Str,
            provides => %RakuProvides,
        },
        prefix => $*CWD,
    ),
    :force,
);

note "installed!";

# vim: ft=perl6
