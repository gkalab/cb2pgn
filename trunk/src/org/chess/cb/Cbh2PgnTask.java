package org.chess.cb;

import android.app.Activity;
import android.app.ProgressDialog;
import android.os.AsyncTask;
import android.util.Log;

public class Cbh2PgnTask extends AsyncTask<String, Void, Integer> {
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

	public Cbh2PgnTask(Activity activity) {
		super();
		this.activity = activity;
		Log.d(TAG, "creating progress dialog");
		this.progressDlg = new ProgressDialog(this.activity);
	}

	protected void onPreExecute() {
		progressDlg.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
		progressDlg.setMessage("Converting...");
		progressDlg.setCancelable(false);
		Log.d(TAG, "showing progress dialog");
		progressDlg.show();
	}

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
	protected Integer doInBackground(String... params) {
		this.fileName = params[0];
		this.outputDir = params[1];

		android.os.Process
				.setThreadPriority(android.os.Process.THREAD_PRIORITY_LESS_FAVORABLE);
		return this.convertToPgn(fileName, outputDir);
	}

	@Override
	protected void onPostExecute(Integer result) {
		if (progressDlg != null && progressDlg.isShowing()) {
			Log.d(TAG, "dismissing progress dialog");
			progressDlg.dismiss();
		}
		if (result > 0) {
			((IConversionCallback) activity).success(result, pgnFileName);
		} else {
			((IConversionCallback) activity).failure();
		}
	}
}
