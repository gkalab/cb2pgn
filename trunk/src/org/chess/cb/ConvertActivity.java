package org.chess.cb;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;

public class ConvertActivity extends Activity implements IConversionCallback {
	private static final String TAG = "CBH2PGN";

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		String fileName = getIntent().getAction();
		List<String> missing = getMissingFiles(fileName);
		if (missing.size() == 0) {
			File pgnDir = new File(Environment.getExternalStorageDirectory()
					+ File.separator + "pgn");
			if (!pgnDir.exists()) {
				if (!pgnDir.mkdirs()) {
					this.setResult(RESULT_CANCELED);
					showError("Unable to create directory "
							+ pgnDir.getAbsolutePath());
				}
			}
			Log.d(TAG, "creating cbh2pgntask");
			new Cbh2PgnTask(this).execute(fileName, pgnDir.getAbsolutePath());
			Log.d(TAG, "cbh2pgntask created");
		} else {
			String message = "The following files are missing:";
			for (String name : missing) {
				message += "\n" + name;
			}
			this.setResult(RESULT_CANCELED);
			showError(message);
		}
	}

	/**
	 * Check if all required Chessbase files are available and return the
	 * missing files.
	 */
	private List<String> getMissingFiles(String fileName) {
		List<String> result = new ArrayList<String>();
		String[] extensions = new String[] { "g", "a", "p", "t", "c", "s" };
		for (String extension : extensions) {
			String name = fileName.substring(0, fileName.length() - 1);
			if (Character.isUpperCase(name.length() - 1)) {
				name = name + extension.toUpperCase();
			} else {
				name = name + extension;
			}
			if (!new File(name).exists()) {
				Log.e(TAG, "Missing file: " + name);
				result.add(name);
			}
		}
		return result;
	}

	private void showError(String message) {
		final AlertDialog d = new AlertDialog.Builder(this)
				.setPositiveButton(android.R.string.ok, new OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
						finish();
					}
				}).setTitle("Error")
				.setIcon(android.R.drawable.ic_dialog_alert)
				.setMessage(message).create();
		d.show();
	}

	@Override
	public void success(int numGames, String fileName) {
		Intent intent = new Intent();
		intent.putExtra("numGames", numGames);
		this.setResult(RESULT_OK, intent.setAction(fileName));
		Log.d(TAG, "success");
		finish();
	}

	@Override
	public void failure() {
		Intent intent = new Intent();
		intent.putExtra("numGames", -1);
		this.setResult(RESULT_CANCELED, intent);
		Log.d(TAG, "failure");
		finish();
	}
}
