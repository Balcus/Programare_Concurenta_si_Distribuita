#include "libconfig.h"  // config_read_file(), config_lookup_string(), config_destroy(), config_init(), config_error_file(), config_error_text(), config_error_line(), config_t
#include <stdio.h>      // printf(), fprintf(), perror(), fgets(), stderr
#include <stdlib.h>     // exit(), malloc(), free()
#include <string.h>     // strcmp(), strdup(), strcspn(), strtok_r()
#include <unistd.h>     // (included for POSIX compliance; no direct calls in this file)
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // waitpid(), WIFEXITED(), WEXITSTATUS()

/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 3: shell.c
 * Se va implementa un shell folosind mecanismul fork() - exec()
*/

#define BUFFER_SIZE 1024
#define MAX_COMMAND_ARGS 32
#define CONFIG_PATH "shell_config.cfg"

struct ParsedConfig {
    char* username;
    char* password;
    char* prompt;
};

struct Command {
    char* name;
    int argc;
    char* *argv;
};

struct ParsedConfig parse_config(config_t *cfg);
void destroy_parsed_config(struct ParsedConfig *pc);
int shell(struct ParsedConfig *pc);
void login(char * const username, char * const password, struct ParsedConfig *pc);
int system_2(char * const command);
int adatped_system(char * const commands);
void init_command(struct Command * command);
struct Command parse_command(char * const command);
void destroy_command(struct Command * command);

struct ParsedConfig parse_config(config_t *cfg) {
    const char *prompt, *username, *password;

    if(!config_read_file(cfg, CONFIG_PATH)) {
        if (fprintf(stderr, "%s:%d - %s\n", config_error_file(cfg), config_error_line(cfg), config_error_text(cfg)) < 0) {
            perror("Eroare la apelul fprintf() ");
        };
        config_destroy(cfg);
        exit(1);
    }

    if(!config_lookup_string(cfg, "prompt", &prompt)) {
        if (fprintf(stderr, "Campul prompt lipseste din configurare\n") < 0) {
            perror("Eroare la apelul fprintf() ");
        }
        config_destroy(cfg);
        exit(1);
    }

    if(!config_lookup_string(cfg, "username", &username)) {
        if (fprintf(stderr, "Campul username lipseste din configurare\n") < 0){
            perror("Eroare la apelul fprintf() ");
        }
        config_destroy(cfg);
        exit(1);
    }

    if(!config_lookup_string(cfg, "password", &password)) {
        if (fprintf(stderr, "Campul password lipseste din configurare\n") < 0) {
            perror("Eroare la apelul fprintf() ");
        }
        config_destroy(cfg);
        exit(1);
    }

    struct ParsedConfig parsed;
    parsed.username = strdup(username);
    parsed.password = strdup(password);
    parsed.prompt = strdup(prompt);

    return parsed;
}

void destroy_parsed_config(struct ParsedConfig *pc) {
    free(pc->username);
    free(pc->password);
    free(pc->prompt);
}

void login(char * const username, char * const password, struct ParsedConfig *pc) {
    if (strcmp(username, pc->username) != 0 || strcmp(password, pc->password) != 0) {
        if (fprintf(stderr, "Credentiale invalide\n") < 0) {
            perror("Eroare la apelul fprintf() ");
        }
        destroy_parsed_config(pc);
        exit(1);
    }
    printf("Credentiale corecte, bine ai venit!\n");
}

int system_2(char * const command) {
    int status;
    struct Command cmd = parse_command(command);
    if (cmd.argc == 0) {
        destroy_command(&cmd);
        return 0;
    }

    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("Eroare la apelul fork() ");
        return -1;
    }

    if (child_pid == 0) {
        execvp(cmd.argv[0], cmd.argv);
        perror("Eroare la executia execvp ");
        exit(-1);
    }

    waitpid(child_pid, &status, 0);
    destroy_command(&cmd);

    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

int adatped_system(char * const line) {
    int last_status = 0;
    char *saveptr;
    const char * sep = ";";
    char *token = strtok_r(line, sep, &saveptr);

    while (token != NULL) {
        while (*token == ' ') {
            token++;
        }

        if (*token != '\0') {
            last_status = system_2(token);
        }

        token = strtok_r(NULL, sep, &saveptr);
    }

    return last_status;
}

struct Command parse_command(char * const command) {
    struct Command cmd;
    init_command(&cmd);

    const char* sep = " ";
    char *saveptr;
    char *token = strtok_r(command, sep, &saveptr);

    while (token != NULL) {
        cmd.argv[cmd.argc] = token;
        cmd.argc++;
        token = strtok_r(NULL, sep, &saveptr);
    }

    cmd.argv[cmd.argc] = NULL;
    if(cmd.argc > 0) {
        cmd.name = cmd.argv[0];
    }

    return cmd;
}

void init_command(struct Command * command) {
    command->name = NULL;
    command->argc = 0;
    command->argv = malloc(MAX_COMMAND_ARGS * sizeof(char*));
}

void destroy_command(struct Command * command) {
    free(command->argv);
}

int shell(struct ParsedConfig *pc) {
    int status = 0;

    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];

    printf("username: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        if (fprintf(stderr, "Eroare la citire username\n") < 0) {
            perror("Eroare la apelul fprintf() ");
        }
        destroy_parsed_config(pc);
        return -1;
    }
    username[strcspn(username, "\n")] = '\0';

    printf("password: ");
    if (fgets(password, sizeof(password), stdin) == NULL) {
        if (fprintf(stderr, "Eroare la citire parola\n") < 0) {
            perror("Eroare la apelul fprintf() ");
        }
        destroy_parsed_config(pc);
        return -1;
    }
    password[strcspn(password, "\n")] = '\0';

    login(username, password, pc);

    char line[BUFFER_SIZE];
    while (1) {
        printf("%s ", pc->prompt);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }
        line[strcspn(line, "\n")] = '\0';
        if (adatped_system(line) != 0) {
            status = -1;
        }
    }
    return status;
}

int main() {
    config_t cfg;
    config_init(&cfg);
    struct ParsedConfig pc = parse_config(&cfg);
    int status = shell(&pc);
    destroy_parsed_config(&pc);
    config_destroy(&cfg);
    return status;
}

/*

root@f944939cbc1b:/app# clang-tidy main.c -- -std=c17
2 warnings and 1 error generated.
Warning-ul nu e o problema deoarece codul a fost verificat de clang-tidy intr-un container docker special pentru asta pe care nu instalez dependintele (rulez codul local nu pe container).
Error while processing /app/main.c.
/app/main.c:1:10: error: 'libconfig.h' file not found [clang-diagnostic-error]
    1 | #include "libconfig.h"
      |          ^~~~~~~~~~~~~
Suppressed 2 warnings (2 in non-user code).
Use -header-filter=.* to display errors from all non-system headers. Use -system-headers to display errors from system headers as well.
Found compiler error(s).

% cat shell_config.cfg
name = "Shell"
version = 1.0;

prompt = "shell>"
username = "balcus",
password = "password"

% ./shell
username: balcus
password: aaa
Credentiale invalide

% ./shell
username: balcus
password: password
Credentiale corecte, bine ai venit!
shell> echo hello
hello
shell> touch file 
shell> vim file
shell> ls;cat file
app1			app4			cmake_install.cmake	compile_commands.json	Makefile
app2			app5			CMakeCache.txt		file			shell
app3			app6			CMakeFiles		generators		shell_config.cfg
hello world
shell> ls -la;tail -n 5 file
total 800
drwxr-xr-x@ 17 balcus  staff    544 Mar 21 13:58 .
drwxr-xr-x@  3 balcus  staff     96 Mar 21 13:39 ..
-rwxr-xr-x@  1 balcus  staff  50296 Mar 21 13:39 app1
-rwxr-xr-x@  1 balcus  staff  50296 Mar 21 13:39 app2
-rwxr-xr-x@  1 balcus  staff  50296 Mar 21 13:39 app3
-rwxr-xr-x@  1 balcus  staff  33720 Mar 21 13:39 app4
-rwxr-xr-x@  1 balcus  staff  50296 Mar 21 13:39 app5
-rwxr-xr-x@  1 balcus  staff  33720 Mar 21 13:39 app6
-rw-r--r--@  1 balcus  staff   2042 Mar 21 13:39 cmake_install.cmake
-rw-r--r--@  1 balcus  staff  14268 Mar 21 13:39 CMakeCache.txt
drwxr-xr-x@ 20 balcus  staff    640 Mar 21 13:39 CMakeFiles
-rw-r--r--@  1 balcus  staff   2992 Mar 21 13:39 compile_commands.json
-rw-r--r--@  1 balcus  staff     12 Mar 21 13:58 file
drwxr-xr-x@ 17 balcus  staff    544 Mar 21 13:39 generators
-rw-r--r--@  1 balcus  staff  11595 Mar 21 13:39 Makefile
-rwxr-xr-x@  1 balcus  staff  77016 Mar 21 13:39 shell
-rw-r--r--@  1 balcus  staff     92 Mar 21 13:39 shell_config.cfg
hello world
shell> pwd
/Users/balcus/Documents/Projects/PCD/teme/tema_3/build/Release
shell> pwd;echo $USER
/Users/balcus/Documents/Projects/PCD/teme/tema_3/build/Release
$USER
shell> cat /etc/passwd;echo test;head -n 4 /dev/null
##
# User Database
# 
# Note that this file is consulted directly only when the system is running
# in single-user mode.  At other times this information is provided by
# Open Directory.
#
# See the opendirectoryd(8) man page for additional information about
# Open Directory.
##
nobody:*:-2:-2:Unprivileged User:/var/empty:/usr/bin/false
root:*:0:0:System Administrator:/var/root:/bin/sh
daemon:*:1:1:System Services:/var/root:/usr/bin/false
_uucp:*:4:4:Unix to Unix Copy Protocol:/var/spool/uucp:/usr/sbin/uucico
_taskgated:*:13:13:Task Gate Daemon:/var/empty:/usr/bin/false
_networkd:*:24:24:Network Services:/var/networkd:/usr/bin/false
_installassistant:*:25:25:Install Assistant:/var/empty:/usr/bin/false
_lp:*:26:26:Printing Services:/var/spool/cups:/usr/bin/false
_postfix:*:27:27:Postfix Mail Server:/var/spool/postfix:/usr/bin/false
_scsd:*:31:31:Service Configuration Service:/var/empty:/usr/bin/false
_ces:*:32:32:Certificate Enrollment Service:/var/empty:/usr/bin/false
_appstore:*:33:33:Mac App Store Service:/var/db/appstore:/usr/bin/false
_mcxalr:*:54:54:MCX AppLaunch:/var/empty:/usr/bin/false
_appleevents:*:55:55:AppleEvents Daemon:/var/empty:/usr/bin/false
_geod:*:56:56:Geo Services Daemon:/var/db/geod:/usr/bin/false
_devdocs:*:59:59:Developer Documentation:/var/empty:/usr/bin/false
_sandbox:*:60:60:Seatbelt:/var/empty:/usr/bin/false
_mdnsresponder:*:65:65:mDNSResponder:/var/empty:/usr/bin/false
_ard:*:67:67:Apple Remote Desktop:/var/empty:/usr/bin/false
_www:*:70:70:World Wide Web Server:/Library/WebServer:/usr/bin/false
_eppc:*:71:71:Apple Events User:/var/empty:/usr/bin/false
_cvs:*:72:72:CVS Server:/var/empty:/usr/bin/false
_svn:*:73:73:SVN Server:/var/empty:/usr/bin/false
_mysql:*:74:74:MySQL Server:/var/empty:/usr/bin/false
_sshd:*:75:75:sshd Privilege separation:/var/empty:/usr/bin/false
_qtss:*:76:76:QuickTime Streaming Server:/var/empty:/usr/bin/false
_cyrus:*:77:6:Cyrus Administrator:/var/imap:/usr/bin/false
_mailman:*:78:78:Mailman List Server:/var/empty:/usr/bin/false
_appserver:*:79:79:Application Server:/var/empty:/usr/bin/false
_clamav:*:82:82:ClamAV Daemon:/var/virusmails:/usr/bin/false
_amavisd:*:83:83:AMaViS Daemon:/var/virusmails:/usr/bin/false
_jabber:*:84:84:Jabber XMPP Server:/var/empty:/usr/bin/false
_appowner:*:87:87:Application Owner:/var/empty:/usr/bin/false
_windowserver:*:88:88:WindowServer:/var/empty:/usr/bin/false
_spotlight:*:89:89:Spotlight:/var/empty:/usr/bin/false
_tokend:*:91:91:Token Daemon:/var/empty:/usr/bin/false
_securityagent:*:92:92:SecurityAgent:/var/db/securityagent:/usr/bin/false
_calendar:*:93:93:Calendar:/var/empty:/usr/bin/false
_teamsserver:*:94:94:TeamsServer:/var/teamsserver:/usr/bin/false
_update_sharing:*:95:-2:Update Sharing:/var/empty:/usr/bin/false
_installer:*:96:-2:Installer:/var/empty:/usr/bin/false
_atsserver:*:97:97:ATS Server:/var/empty:/usr/bin/false
_ftp:*:98:-2:FTP Daemon:/var/empty:/usr/bin/false
_unknown:*:99:99:Unknown User:/var/empty:/usr/bin/false
_softwareupdate:*:200:200:Software Update Service:/var/db/softwareupdate:/usr/bin/false
_coreaudiod:*:202:202:Core Audio Daemon:/var/empty:/usr/bin/false
_screensaver:*:203:203:Screensaver:/var/empty:/usr/bin/false
_locationd:*:205:205:Location Daemon:/var/db/locationd:/usr/bin/false
_trustevaluationagent:*:208:208:Trust Evaluation Agent:/var/empty:/usr/bin/false
_timezone:*:210:210:AutoTimeZoneDaemon:/var/empty:/usr/bin/false
_lda:*:211:211:Local Delivery Agent:/var/empty:/usr/bin/false
_cvmsroot:*:212:212:CVMS Root:/var/empty:/usr/bin/false
_usbmuxd:*:213:213:iPhone OS Device Helper:/var/db/lockdown:/usr/bin/false
_dovecot:*:214:6:Dovecot Administrator:/var/empty:/usr/bin/false
_dpaudio:*:215:215:DP Audio:/var/empty:/usr/bin/false
_postgres:*:216:216:PostgreSQL Server:/var/empty:/usr/bin/false
_krbtgt:*:217:-2:Kerberos Ticket Granting Ticket:/var/empty:/usr/bin/false
_kadmin_admin:*:218:-2:Kerberos Admin Service:/var/empty:/usr/bin/false
_kadmin_changepw:*:219:-2:Kerberos Change Password Service:/var/empty:/usr/bin/false
_devicemgr:*:220:220:Device Management Server:/var/empty:/usr/bin/false
_webauthserver:*:221:221:Web Auth Server:/var/empty:/usr/bin/false
_netbios:*:222:222:NetBIOS:/var/empty:/usr/bin/false
_warmd:*:224:224:Warm Daemon:/var/empty:/usr/bin/false
_dovenull:*:227:227:Dovecot Authentication:/var/empty:/usr/bin/false
_netstatistics:*:228:228:Network Statistics Daemon:/var/empty:/usr/bin/false
_avbdeviced:*:229:-2:Ethernet AVB Device Daemon:/var/empty:/usr/bin/false
_krb_krbtgt:*:230:-2:Open Directory Kerberos Ticket Granting Ticket:/var/empty:/usr/bin/false
_krb_kadmin:*:231:-2:Open Directory Kerberos Admin Service:/var/empty:/usr/bin/false
_krb_changepw:*:232:-2:Open Directory Kerberos Change Password Service:/var/empty:/usr/bin/false
_krb_kerberos:*:233:-2:Open Directory Kerberos:/var/empty:/usr/bin/false
_krb_anonymous:*:234:-2:Open Directory Kerberos Anonymous:/var/empty:/usr/bin/false
_assetcache:*:235:235:Asset Cache Service:/var/empty:/usr/bin/false
_coremediaiod:*:236:236:Core Media IO Daemon:/var/empty:/usr/bin/false
_launchservicesd:*:239:239:_launchservicesd:/var/empty:/usr/bin/false
_iconservices:*:240:240:IconServices:/var/empty:/usr/bin/false
_distnote:*:241:241:DistNote:/var/empty:/usr/bin/false
_nsurlsessiond:*:242:242:NSURLSession Daemon:/var/db/nsurlsessiond:/usr/bin/false
_displaypolicyd:*:244:244:Display Policy Daemon:/var/empty:/usr/bin/false
_astris:*:245:245:Astris Services:/var/db/astris:/usr/bin/false
_krbfast:*:246:-2:Kerberos FAST Account:/var/empty:/usr/bin/false
_gamecontrollerd:*:247:247:Game Controller Daemon:/var/empty:/usr/bin/false
_mbsetupuser:*:248:248:Setup User:/var/setup:/bin/bash
_ondemand:*:249:249:On Demand Resource Daemon:/var/db/ondemand:/usr/bin/false
_xserverdocs:*:251:251:macOS Server Documents Service:/var/empty:/usr/bin/false
_wwwproxy:*:252:252:WWW Proxy:/var/empty:/usr/bin/false
_mobileasset:*:253:253:MobileAsset User:/var/ma:/usr/bin/false
_findmydevice:*:254:254:Find My Device Daemon:/var/db/findmydevice:/usr/bin/false
_datadetectors:*:257:257:DataDetectors:/var/db/datadetectors:/usr/bin/false
_captiveagent:*:258:258:captiveagent:/var/empty:/usr/bin/false
_ctkd:*:259:259:ctkd Account:/var/db/ctkd:/usr/bin/false
_applepay:*:260:260:applepay Account:/var/db/applepay:/usr/bin/false
_hidd:*:261:261:HID Service User:/var/db/hidd:/usr/bin/false
_cmiodalassistants:*:262:262:CoreMedia IO Assistants User:/var/db/cmiodalassistants:/usr/bin/false
_analyticsd:*:263:263:Analytics Daemon:/var/db/analyticsd:/usr/bin/false
_fpsd:*:265:265:FPS Daemon:/var/db/fpsd:/usr/bin/false
_timed:*:266:266:Time Sync Daemon:/var/db/timed:/usr/bin/false
_nearbyd:*:268:268:Proximity and Ranging Daemon:/var/db/nearbyd:/usr/bin/false
_reportmemoryexception:*:269:269:ReportMemoryException:/var/db/reportmemoryexception:/usr/bin/false
_driverkit:*:270:270:DriverKit:/var/empty:/usr/bin/false
_diskimagesiod:*:271:271:DiskImages IO Daemon:/var/db/diskimagesiod:/usr/bin/false
_logd:*:272:272:Log Daemon:/var/db/diagnostics:/usr/bin/false
_appinstalld:*:273:273:App Install Daemon:/var/db/appinstalld:/usr/bin/false
_installcoordinationd:*:274:274:Install Coordination Daemon:/var/db/installcoordinationd:/usr/bin/false
_demod:*:275:275:Demo Daemon:/var/empty:/usr/bin/false
_rmd:*:277:277:Remote Management Daemon:/var/db/rmd:/usr/bin/false
_accessoryupdater:*:278:278:Accessory Update Daemon:/var/db/accessoryupdater:/usr/bin/false
_knowledgegraphd:*:279:279:Knowledge Graph Daemon:/var/db/knowledgegraphd:/usr/bin/false
_coreml:*:280:280:CoreML Services:/var/db/coreml:/usr/bin/false
_sntpd:*:281:281:SNTP Server Daemon:/var/empty:/usr/bin/false
_trustd:*:282:282:trustd:/var/empty:/usr/bin/false
_mmaintenanced:*:283:283:mmaintenanced:/var/db/mmaintenanced:/usr/bin/false
_darwindaemon:*:284:284:Darwin Daemon:/var/db/darwindaemon:/usr/bin/false
_notification_proxy:*:285:285:Notification Proxy:/var/empty:/usr/bin/false
_avphidbridge:*:288:288:Apple Virtual Platform HID Bridge:/var/empty:/usr/bin/false
_biome:*:289:289:Biome:/var/db/biome:/usr/bin/false
_backgroundassets:*:291:291:Background Assets Service:/var/empty:/usr/bin/false
_mobilegestalthelper:*:293:293:MobileGestaltHelper:/var/empty:/usr/bin/false
_audiomxd:*:294:294:Audio and MediaExperience Daemon:/var/db/audiomxd:/usr/bin/false
_terminusd:*:295:295:Terminus:/var/db/terminus:/usr/bin/false
_neuralengine:*:296:296:AppleNeuralEngine:/var/db/neuralengine:/usr/bin/false
_eligibilityd:*:297:297:OS Eligibility Daemon:/var/db/eligibilityd:/usr/bin/false
_systemstatusd:*:298:298:SystemStatus Services:/var/empty:/usr/bin/false
_aonsensed:*:300:300:Always On Sense Daemon:/var/db/aonsensed:/usr/bin/false
_modelmanagerd:*:301:301:Model Manager:/var/db/modelmanagerd:/usr/bin/false
_reportsystemmemory:*:302:302:ReportSystemMemory:/var/empty:/usr/bin/false
_swtransparencyd:*:303:303:Software Transparency Services:/var/db/swtransparencyd:/usr/bin/false
_naturallanguaged:*:304:304:Natural Language Services:/var/db/com.apple.naturallanguaged:/usr/bin/false
_spinandd:*:305:305:SPINAND Daemon:/var/empty:/usr/bin/false
_corespeechd:*:306:306:CoreSpeech Services:/var/empty:/usr/bin/false
_diagnosticservicesd:*:307:307:Diagnostic Services:/var/empty:/usr/bin/false
_mds_stores:*:308:308:Spotlight File Metadata Index Daemon:/var/empty:/usr/bin/false
_oahd:*:441:441:OAH Daemon:/var/empty:/usr/bin/false
test
shell> ^C

*/