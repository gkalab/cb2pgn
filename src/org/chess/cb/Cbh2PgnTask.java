package org.chess.cb;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.pm.ActivityInfo;
import android.os.AsyncTask;
import android.util.Log;

public class Cbh2PgnTask extends AsyncTask {
	private static final String TAG = "CBH2PGN";
	private Activity activity;
	private ProgressDialog progressDlg;
	private String fileName;
	private String outputDir;
	private String pgnFileName;
	private int orientation;

	static {
		System.loadLibrary("zlib");
		System.loadLibrary("minizip");
		System.loadLibrary("mstl");
		System.loadLibrary("universalchardet");
		System.loadLibrary("zzip");
		System.loadLibrary("util");
		System.loadLibrary("db");
		System.loadLibrary("nativecb");
	}

	/** Convert from .cbh to .pgn */
	private final native int convertToPgn(String fileName, String outputDir);

	private void progress(final int progress) {
		this.activity.runOnUiThread(new Runnable() {
			public void run() {
				progressDlg.setProgress(progress);
			}
		});
	}

	private void setNOGames(final int numGames) {
		Log.d(TAG, "setting max number of games to: " + numGames);
		progressDlg.setMax(numGames);
	}

	private void setPgnFileName(final String pgnFileName) {
		Log.d(TAG, "PGN file name: " + pgnFileName);
		this.pgnFileName = pgnFileName;
	}

	@Override
	protected Object doInBackground(Object... params) {
		this.activity = (Activity) params[0];
		this.fileName = (String) params[1];
		this.outputDir = (String) params[2];
		this.progressDlg = (ProgressDialog) params[3];

		disableOrientationChange();
		android.os.Process
				.setThreadPriority(android.os.Process.THREAD_PRIORITY_LESS_FAVORABLE);
		return this.convertToPgn(fileName, outputDir);
	}

	private void enableOrientationChange() {
		activity.setRequestedOrientation(orientation);
	}

	private void disableOrientationChange() {
		this.orientation = activity.getRequestedOrientation();
		activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_NOSENSOR);
	}

	@Override
	protected void onPostExecute(Object result) {
		progressDlg.dismiss();
		enableOrientationChange();
		Integer resultValue = (Integer) result;
		if (resultValue > 0) {
			((IConversionCallback) activity).success(resultValue, pgnFileName);
		} else {
			((IConversionCallback) activity).failure();
		}
	}

}
