import subprocess
class ChessEngine:
	def __init__(A,engine_path):A.engine=subprocess.Popen([engine_path],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True);A._initialize_engine()
	def _initialize_engine(A):
		A._send_command('uci')
		while True:
			B=A._read_output()
			if B=='uciok':break
		A._send_command('setoption name Hash value 1')
	def _send_command(A,command):'Send a command to the engine.';A.engine.stdin.write(command+'\n');A.engine.stdin.flush()
	def _read_output(A):'Read a single line of output from the engine.';B=A.engine.stdout.readline().strip();return B
	def get_best_move(A,fen,movetime=100):
		'Get the best move for a given position.';A._send_command(f"position fen {fen}");A._send_command(f"go movetime {movetime}");B=None
		while True:
			C=A._read_output()
			if C.startswith('bestmove'):B=C.split()[1];break
		A._send_command('setoption name Clear Hash');return B
	def stop(A):'Stop the engine process.';A._send_command('quit');A.engine.terminate();A.engine.wait()
ultima=None
def chess_bot(obs):
	global ultima;A=obs['board'];'\n    \n    Comment engine_path with /kaggle_simulations/...  and %%writefile to define func and test locally or \n    comment engine_path with /kaggle/working/ ... to save file and then zip it\n    ';B='/kaggle_simulations/agent/stockfish'
	if ultima is None:ultima=ChessEngine(B)
	C=ultima.get_best_move(A);return C