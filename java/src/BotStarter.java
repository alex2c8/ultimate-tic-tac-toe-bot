import java.util.ArrayList;

public class BotStarter {
    /**
     * Makes a turn. Edit this method to make your bot smarter.
     *
     * @return The column where the turn was made.
     */
    public static Move makeTurn(Field field) {
        //  get the available moves for this game's state
        ArrayList<Move> moves = field.getAvailableMoves();

        Move nextMove = null;

        //  make the first move in the center
        if (field.getMoveNr() == 1) {
            return new Move(4, 4);
        }

        //  if I have to move in an empty square, do it so the opponent's move
        //  will be in the same square -> better control
        if (isEmptySquare(moves)) {
            Move aux = moves.get(0);
            return new Move(4 * (aux.getX() / 3), 4 * (aux.getY() / 3));
        }

        //  if there is a square that can be won directly
        //  that will lead to the end of the game in my favor
        if (field.multipleActiveSquares()) {
            if ((nextMove = field.tryToWinGame(moves, BotParser.mBotId)) != null)
                return nextMove;
        }


        //  get best move using minimax
        long score = Integer.MIN_VALUE;
        long alpha = Integer.MIN_VALUE;
        long beta = Integer.MAX_VALUE;

        //  dynamically compute depth, based upon the number of available moves
        int depth = getDepth(moves.size(), field);

        //  clone current field in order to be able to simulate moves
        Field clone = Field.clone(field);

        //  for each available move
        for (Move m : moves) {
            //  simulate current move
            clone.simulateMove(m, BotParser.mBotId);

            //  calculate this move's score
            long currentScore = minimax(clone, depth, alpha, beta, BotParser.mEnemyId);

            //  undo this move
            clone.undoMove(m, field);

            //  update general score and move
            if (currentScore > score) {
                score = currentScore;
                nextMove = m;
            }
        }

        // System.err.println(nextMove + " of score: " + score + ", depth: " + depth);
        return nextMove;
    }


    //  dynamically compute depth
    //  depth is inversely proportional to the moves' size
    public static int getDepth(int movesSize, Field field) {
        //  for initial moves, set the depth to 4
        if (field.getMoveNr() < 18 && movesSize > 7)
            return 4;

        int depth = 7;

        if (movesSize == 5)
            depth = 8;

        else if (movesSize < 5)
            depth = 9;


        if (movesSize > 7 && movesSize <= 10)
            depth = 5;

        else if (movesSize > 10 && movesSize <= 17)
            depth = 4;

        else if (movesSize > 17 && movesSize <= 46)
            depth = 3;

        else if (movesSize > 46)
            depth = 2;


        if (BotParser.mTimeLeft < 4000 && depth > 3)
            depth = 3;

        else if (BotParser.mTimeLeft < 2000)
            depth = 1;


        return depth;
    }


    //  minimax + alpha-beta pruning
    public static long minimax(Field field, int depth, long alpha, long beta, int player) {
        if (depth == 0 || field.isDone()) {
            return field.evaluate(BotParser.mBotId) - field.evaluate(BotParser.mEnemyId);
        }

        long score;

        ArrayList<Move> moves = field.getAvailableMoves();
        int movesSize = moves.size();

        //  compute depth
        if (movesSize > 7) {
            int newDepth;

            if (movesSize <= 10)
                newDepth = 5;

            else if (movesSize > 10 && movesSize <= 17)
                newDepth = 4;

            else if (movesSize > 17 && movesSize <= 46)
                newDepth = 3;

            else
                newDepth = 2;

            depth = Math.min(depth, newDepth);
        }

        //  clone current field
        Field clone = Field.clone(field);

        // my turn
        if (player == BotParser.mBotId) {
            //  start pessimistic
            score = Integer.MIN_VALUE;

            //  for each available simulateMove
            for (Move m : moves) {

                //  simulate current simulateMove m
                clone.simulateMove(m, BotParser.mBotId);

                //  calculate this simulateMove's score
                long currentScore = minimax(clone, depth - 1, alpha, beta, BotParser.mEnemyId);

                //  undo current simulateMove
                clone.undoMove(m, field);

                //  update scores
                if (currentScore > score) {
                    score = currentScore;
                    alpha = score;

                    //  pruning
                    if (alpha >= beta)
                        return score;
                }
            }
        }

        //  enemy's turn
        else {
            //  start pessimistic
            score = Integer.MAX_VALUE;

            //  for each available simulateMove
            for (Move m : moves) {

                //  simulate current enemy's simulateMove
                clone.simulateMove(m, BotParser.mEnemyId);

                //  calculate this simulateMove's score
                long currentScore = minimax(clone, depth - 1, alpha, beta, BotParser.mBotId);

                //  undo current simulateMove
                clone.undoMove(m, field);

                //  update scores
                if (currentScore < score) {
                    score = currentScore;
                    beta = score;

                    //  pruning
                    if (alpha >= beta)
                        return score;
                }
            }
        }

        return score;
    }


    //  checks whether a square is empty by looking at the available moves
    public static boolean isEmptySquare(ArrayList<Move> moves) {
        if (moves.size() != 9) {
            return false;
        }
        //  first and last indices must differ by 2
        else {
            return ((moves.get(0).getX() + 2 == moves.get(8).getX()) &&
                    (moves.get(0).getY() + 2 == moves.get(8).getY()));
        }
    }


    public static void main(String[] args) {
        BotParser parser = new BotParser(new BotStarter());
        parser.run();

//        Field field = new Field();
//
//        int[][] macro = {
//                {2, -1, 2},
//                {2, 2, -1},
//                {1, 1, 0}
//        };
//
//        int[][] board = {
//                {1, 1, 2, 2, 0, 0, 0, 0, 1},
//                {2, 2, 2, 0, 0, 2, 0, 2, 2},
//                {0, 0, 0, 1, 1, 0, 0, 0, 0},
//                {0, 1, 2, 2, 2, 2, 2, 1, 0},
//                {1, 0, 0, 2, 1, 2, 0, 1, 1},
//                {0, 0, 0, 2, 0, 2, 0, 1, 0},
//                {0, 0, 0, 2, 0, 0, 2, 2, 1},
//                {0, 1, 0, 0, 2, 0, 1, 1, 2},
//                {1, 0, 2, 1, 1, 1, 2, 1, 2}
//        };
//
//        field.mMacroboard = macro;
//        field.mBoard = board;
//
//        field.mMoveNr = 40;
//
//        long x = System.nanoTime();
//        System.out.println(makeTurn(field));
//
//        System.out.println((System.nanoTime() - x) * 1e-9);
    }
}
