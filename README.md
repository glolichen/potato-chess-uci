# Potato Chess

UCI compatible chess engine. Also available as a [lichess bot](https://lichess.org/@/potato-chess-bot/).

## Features
 * Bitboard move generation
 * Principal Variation search
 * Quiscence search
 * Transposition table w/ Zobrist Hashing
 * Polyglot opening book support
 * Multithreaded perft and search
 * Pondering

## Acknowledgements
 * The [lichess bot developers](https://github.com/lichess-bot-devs), for their project [lichess-bot](https://github.com/lichess-bot-devs/lichess-bot) that made the lichess bot possible
 * Fabien Letouzey and his [Fruit engine](https://arctrix.com/nas/chess/fruit/), whose Polyglot probing code is used in this program
   * The program is licensed under GPL-v2 to comply with Fruit 2.1's license
 * The [Leela Chess Zero developers](https://github.com/LeelaChessZero/) for providing the [time usage curve](https://lczero.org/blog/2018/09/time-management/) that is also used in this program
 * H. G. Muller for [documenting](http://hgm.nubati.net/book_format.html) how Polyglot hashing works
   * The algorithm he described is used in this program
 * [The Chess Programming Wiki](https://chessprogramming.org/) whose documentation has helped greatly in the development of this program
 * [Ronald Friederich's RofChade engine](https://www.chessprogramming.org/RofChade) for providing the piece location values used in the evaluation function
 * [vit-vit](https://github.com/vit-vit)'s [CTPL library](https://github.com/vit-vit/CTPL), which powers this program's multithreaded search function