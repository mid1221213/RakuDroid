use lib <rakudo/lib>;
use CompUnit::Repository::Staging;

my %provides = 'tools/provides'.IO.lines.split(/\s+/);

PROCESS::<$REPO> := CompUnit::Repository::Staging.new(
    :prefix(@*ARGS[0]),
    :next-repo(
        # Make CompUnit::Repository::Staging available to precomp processes
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
            name     => 'RakuDroid',
            auth     => 'Midnite',
            ver      => $*PERL.version.Str,
            provides => %provides,
        },
        prefix => $*CWD,
    ),
    :force,
);

note "installed!";

# vim: ft=perl6
