package org.chess.cb;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.os.Bundle;

public class MainActivity extends Activity {

	private static final int CONVERT_RESULT = 0;
	private static final int RESULT_GET_FILENAME = 1;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		Intent intent = new Intent(this, SelectFileActivity.class);
		intent.setAction("cbh");
		startActivityForResult(intent, RESULT_GET_FILENAME);
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		if (data != null) {
			switch (requestCode) {
			case CONVERT_RESULT:
				if (resultCode == RESULT_OK) {
					String fileName = data.getAction();
					Bundle bundle = data.getExtras();
					int numGames = bundle.getInt("numGames");
					success(numGames, fileName);
				} else {
					failure();
				}
				break;
			case RESULT_GET_FILENAME:
				if (resultCode == RESULT_OK) {
					String fileName = data.getAction();
					if (fileName != null) {
						Intent intent = new Intent(this, ConvertActivity.class);
						intent.setAction(fileName);
						startActivityForResult(intent, CONVERT_RESULT);
					}

				} else {
					this.setResult(RESULT_CANCELED);
					finish();
				}
				break;
			}
		} else {
			finish();
		}
	}

	private void success(int numGames, String fileName) {
		String message = getString(R.string.export_success, "" + numGames)
				+ fileName;
		showResultDialog("Success", message, android.R.drawable.ic_dialog_info);
	}

	private void failure() {
		showResultDialog("Failure", getString(R.string.export_failure),
				android.R.drawable.ic_dialog_alert);
	}

	private void showResultDialog(String title, String message, int icon) {
		final AlertDialog d = new AlertDialog.Builder(this)
				.setPositiveButton(android.R.string.ok, new OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
						finish();
					}
				}).setTitle(title).setIcon(icon).setMessage(message).create();
		d.show();
	}
}
