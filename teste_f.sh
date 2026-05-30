#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

PROG="./codexion"

print_usage() {
	echo "Usage : ./tester.sh <nom-du-test|all|list> [plain|mem|helgrind]"
	echo
	echo "Exemples :"
	echo "  ./tester.sh basic_fifo"
	echo "  ./tester.sh cooldown_impact"
	echo "  ./tester.sh starvation_edf"
	echo "  ./tester.sh all"
	echo "  ./tester.sh one_fifo mem"
}

print_notes() {
	echo -e "${BLUE}Ce qu'il faut verifier :${NC}"
	echo "  - chaque 'is compiling' doit arriver apres deux 'has taken a dongle'"
	echo "  - deux codeurs voisins ne doivent jamais compiler en meme temps"
	echo "  - si le cooldown augmente, la duree totale doit aussi augmenter"
	echo "  - fifo et edf ne doivent pas toujours se comporter exactement pareil"
	echo
}

run_test() {
	name="$1"
	args="$2"
	note="$3"

	echo -e "${GREEN}[$name]${NC}"
	echo "args : $args"
	echo "note : $note"
	echo
	$RUNNER $PROG $args
	echo
}

run_named_test() {
	case "$1" in
		basic_fifo)
			run_test "basic_fifo" \
				"4 800 200 200 200 5 10 fifo" \
				"cas FIFO simple qui doit bien se terminer"
			;;
		basic_edf)
			run_test "basic_edf" \
				"4 800 200 200 200 5 10 edf" \
				"cas EDF simple qui doit bien se terminer"
			;;
		one_fifo)
			run_test "one_fifo" \
				"1 1000 200 200 200 5 50 fifo" \
				"un seul codeur ne peut pas obtenir deux dongles distincts, il doit burnout vers 1000 ms"
			;;
		one_edf)
			run_test "one_edf" \
				"1 1000 200 200 200 5 50 edf" \
				"meme idee que one_fifo mais avec EDF"
			;;
		quota_stop)
			run_test "quota_stop" \
				"5 2000 100 100 100 20 1 fifo" \
				"doit s'arreter parce que tout le monde a atteint le quota, pas a cause d'un burnout"
			;;
		cooldown_impact)
			run_test "cooldown_impact" \
				"5 2000 100 100 100 20 200 fifo" \
				"meme forme que quota_stop mais le cooldown doit ralentir visiblement l'execution"
			;;
		starvation_fifo)
			run_test "starvation_fifo" \
				"3 1000 600 10 10 5 100 fifo" \
				"cas serre, FIFO peut etre plus dur"
			;;
		starvation_edf)
			run_test "starvation_edf" \
				"3 1000 600 10 10 5 100 edf" \
				"cas serre, EDF doit favoriser le codeur le plus urgent"
			;;
		bonus_edf)
			run_test "bonus_edf" \
			"4 2000 200 100 100 3 20 edf" \
			"cas simple sur un ms court"
			;;
		immediate_burnout)
			run_test "immediate_burnout" \
				"2 1 200 200 200 5 10 fifo" \
				"quelqu'un doit burnout presque immediatement"
			;;
		long_actions)
			run_test "long_actions" \
				"3 10000 2000 2000 2000 2 100 fifo" \
				"actions longues, utile pour observer la precision du monitor et l'arret propre"
			;;
		big)
			run_test "big" \
				"100 10000 66 24 87 10 10 fifo" \
				"gros cas parallele, doit finir dans un temps raisonnable"
			;;
		max_coders)
			run_test "max_coders" \
				"300 10000 100 100 100 5 10 edf" \
				"test de charge pour le nombre de threads et le scheduler"
			;;
		error_arg1)
			run_test "error_arg1" \
				"banana 200 300 400 500 5 10 fifo" \
				"premier argument invalide"
			;;
		error_arg2)
			run_test "error_arg2" \
				"10 banana 300 400 500 5 10 fifo" \
				"deuxieme argument invalide"
			;;
		error_arg3)
			run_test "error_arg3" \
				"10 200 banana 400 500 5 10 fifo" \
				"troisieme argument invalide"
			;;
		error_arg4)
			run_test "error_arg4" \
				"10 200 300 banana 500 5 10 fifo" \
				"quatrieme argument invalide"
			;;
		error_arg5)
			run_test "error_arg5" \
				"10 200 300 400 banana 5 10 fifo" \
				"cinquieme argument invalide"
			;;
		error_arg6)
			run_test "error_arg6" \
				"10 200 300 400 500 banana 10 fifo" \
				"sixieme argument invalide"
			;;
		error_arg7)
			run_test "error_arg7" \
				"10 200 300 400 500 5 banana fifo" \
				"septieme argument invalide"
			;;
		error_arg8)
			run_test "error_arg8" \
				"10 200 300 400 500 5 10 banana" \
				"scheduler invalide"
			;;
		error_arg9)
			run_test "error_arg9" \
				"10 200 300 -400 500 5 10 edf" \
				"argument negatif"
			;;
		error_arg10)
			run_test "error_arg10" \
				"too 10 200 300 400 500 5 10 edf" \
				"mauvais nombre d'arguments et premier champ invalide"
			;;
		*)
			echo -e "${RED}Test inconnu : $1${NC}"
			echo
			print_usage
			exit 1
			;;
	esac
}

run_all() {
	run_named_test basic_fifo
	run_named_test basic_edf
	run_named_test one_fifo
	run_named_test quota_stop
	run_named_test cooldown_impact
	run_named_test starvation_fifo
	run_named_test starvation_edf
	run_named_test big
}

list_tests() {
	echo "basic_fifo"
	echo "basic_edf"
	echo "one_fifo"
	echo "one_edf"
	echo "quota_stop"
	echo "cooldown_impact"
	echo "starvation_fifo"
	echo "starvation_edf"
	echo "immediate_burnout"
	echo "long_actions"
	echo "big"
	echo "max_coders"

	echo "error_arg1"
	echo "error_arg2"
	echo "error_arg3"
	echo "error_arg4"
	echo "error_arg5"
	echo "error_arg6"
	echo "error_arg7"
	echo "error_arg8"
	echo "error_arg9"
	echo "error_arg10"
}

if [ ! -x "$PROG" ]; then
	echo -e "${RED}Executable manquant : $PROG${NC}"
	echo "Lance d'abord 'make'."
	exit 1
fi

MODE="${2:-plain}"
case "$MODE" in
	plain)
		RUNNER=""
		;;
	mem)
		RUNNER="valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes"
		;;
	helgrind)
		RUNNER="valgrind --tool=helgrind"
		;;
	*)
		echo -e "${RED}Mode inconnu : $MODE${NC}"
		echo "Utilise plain, mem ou helgrind."
		exit 1
		;;
esac

if [ $# -lt 1 ]; then
	print_usage
	echo
	print_notes
	exit 1
fi

case "$1" in
	help|-h|--help)
		print_usage
		echo
		print_notes
		;;
	list)
		list_tests
		;;
	all)
		print_notes
		run_all
		;;
	*)
		print_notes
		run_named_test "$1"
		;;
esac
