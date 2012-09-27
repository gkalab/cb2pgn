package org.chess.cb;

public interface IConversionCallback {
	void success(int numGames, String fileName);

	void failure();
}
