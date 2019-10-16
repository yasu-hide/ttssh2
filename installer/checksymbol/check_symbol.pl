#! /usr/bin/perl

#
# ���W���[���̈ˑ�API�𒲂ׂ�
#
# Usage(ActivePerl):
#  perl check_symbol.pl <module path>
#

my $g_module;
my $g_outfile = 'toolresult.txt';
my @g_module_symbols;

# �`�F�b�N�ΏۂƂȂ郂�W���[�����ˑ����Ă���DLL
my @g_dlllist = (
	"WS2_32",
	"GDI32",
	"SHELL32",
	"comdlg32",
	"ADVAPI32",
	"ole32",
	"KERNEL32",
	"DNSAPI",
	"USER32",
	"IMM32"
);

# OS���Ƃ�API�ꗗ	
# NirSoft��DLL Export Viewer�ō쐬����
# https://www.nirsoft.net/utils/dll_export_viewer.html
my %g_apilist = (
	'Windows95' => 'api\\win95_dll.txt',
	'Windows98' => 'api\\win98_dll.txt',
	'WindowsMe' => 'api\\winMe_dll.txt',
	'WindowsNT4.0' => 'api\\winnt4_dll.txt',
	'Windows2000' => 'api\\win2000_dll.txt',
	'WindowsXP' => 'api\\winXP_dll.txt',
);

# Windows95�Ŗ��Ȃ�API�̈ꗗ
my @g_whitelist_win95 = (
	"CommDlgExtendedError",
	"GetSaveFileNameA",
	"GetOpenFileNameA",
	"DnsQuery_A",
	"DnsRecordListFree",
	"GetMonitorInfoA",
	"MonitorFromWindow"
);

if (@ARGV != 1) {
	print "ERROR: no argument!\n";
	help();
	exit(1);
}

$g_module = $ARGV[0];
print "���W���[�� $g_module ����͂��܂�...\n\n";

check_tool_existence();
run_tool();
extract_symbol_table();
verify_api_list();

exit(0);

# �R�}���h���C���w���v�̕\��
sub help {
	print <<'EOD';
Usage: check_symbol.pl <module path>

Checking Win32 API included in module.
EOD
}

# �c�[���̃`�F�b�N
sub check_tool_existence {
	if (!(-e "dependencies/Dependencies.exe")) {
		print <<'EOD';
Dependencies �c�[������������Ă��܂���B
���L�T�C�g������肵�āudependencies�v�t�H���_�Ɋi�[���Ă��������B
https://github.com/lucasg/Dependencies		
EOD
		exit(1);
	}
}

# �c�[���̎��s
sub run_tool {
	my($cmd);
	
	$cmd = "dependencies\\Dependencies.exe -imports $g_module > $g_outfile";
	print "$cmd\n";
	system($cmd);
}

sub is_target_dll {
	my($name) = @_;
	
	foreach my $s (@g_dlllist) {
		return 1 if ($s eq $name);
	}
	return 0;
}

# �V���{���̒��o
sub extract_symbol_table {
	local(*FP);
	my($line, $func, $dll);
	
	@g_module_symbols = ();
	
	open(FP, "<$g_outfile") || return;

	$dll = '';
	while ($line = <FP>) {
		chomp($line);
		
		# DLL���̎擾
		if ($line =~ /Import from module (.+).dll/) {
			$dll = $1;		
			#print "$dll.DLL\n";
			if (is_target_dll($dll)) {
				print "$dll.DLL\n";				
			} else {
				$dll = '';		
				next;		
			}
		}
		
		next if ($dll eq '');
		
		#print "$line\n";
		if ($line =~ /Function (\w+)/) {
			$func = $1;
			#print "$func\n";
			push(@g_module_symbols, $func);
		}
	}
	close(FP);
}

# API�̈ˑ��`�F�b�N
sub verify_api_list {
	local(*FP);
	my($key, $os, $file, $api);
	my(@whole_file, @match);	
	my($notfound) = 0;
	
	# OS���Ƃ�API�ꗗ�����ԂɊm�F����
	for $key (keys %g_apilist) {
		$os = $key;
		$file = $g_apilist{$key};
		print "[$os, $file]\n\n";
		
		open(FP, "<$file") || next;
		@whole_file = <FP>;	
		close(FP);
		
		# ���W���[���̈ˑ�API���`�F�b�N����
		foreach $api (@g_module_symbols) {
			#print "API($api)\n";
#			if (excluded_check_api($api)) {
#				next;
#			}
			
			@match = grep(/$api/, @whole_file);
			if (@match == 0) {
				print "$api\n";
				$notfound = 1;
				#last;
			}
		}		
		
		if ($notfound) {
			$notfound = 0;
			print "\n$os �ł͏�LAPI�����݂��Ȃ��̂� Tera Term ���N�����Ȃ��\\��������܂�\n";
		} else {
			print "\n$os �ł͖�肠��܂���\n";
		}
		print "\n\n";
	}
}

# �`�F�b�N�ΏۊO��API��
sub excluded_check_api {
	my($arg) = @_;
	my($s);
	
	foreach $s (@g_whitelist_win95) {
		if ($s eq $arg) {
			#print "$arg API is excluded.\n";
			return 1;
		}
	}
	return 0
}
