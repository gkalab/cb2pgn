package org.chess.cb;

import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Stack;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.text.TextUtils.TruncateAt;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

public class SelectFileActivity extends ListActivity {

	public static final String PARENT_FOLDER = ".. (parent folder)";
	private static Stack<String> path = new Stack<String>();
	private ArrayAdapter<String> listAdapter;
	private String extension;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.filelist);
		View title = findViewById(android.R.id.title);
		if (title instanceof TextView) {
			TextView titleText = (TextView) title;
			titleText.setEllipsize(TruncateAt.START);
		}
		Intent i = getIntent();
		String extension = i.getAction();
		if (extension != null && extension.length() != 0) {
			this.extension = extension;
		} else {
			this.extension = "*";
		}
		String lastPathKey = "lastPath" + extension;
		getPath(lastPathKey);
		final SelectFileActivity fileList = this;
		fileList.showList(lastPathKey);
	}

	private void getPath(String lastPathKey) {
		Stack<String> defaultPath = new Stack<String>();
		defaultPath.push(Environment.getExternalStorageDirectory()
				+ File.separator);
		path = Tools.getStringStackPref(this, lastPathKey, defaultPath);
	}

	protected void showList(final String lastPathKey) {
		listAdapter = new FileListArrayAdapter(this,
				R.layout.select_file_list_item, R.id.select_file_label,
				new ArrayList<String>());
		setListAdapter(listAdapter);

		List<String> fileNames = changePath();

		if (path.size() == 0 && fileNames.size() == 0) {
			AlertDialog.Builder builder = new AlertDialog.Builder(this);
			String msg = getString(R.string.no_files, "/");
			builder.setTitle(R.string.app_name).setMessage(msg);
			builder.setPositiveButton(android.R.string.ok,
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int which) {
							setResult(Activity.RESULT_CANCELED);
							finish();
							return;
						}
					});
			AlertDialog alert = builder.create();
			alert.show();
		}

		ListView lv = getListView();
		lv.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View view, int pos,
					long id) {
				if (pos >= 0 && pos < listAdapter.getCount()) {
					String item = listAdapter.getItem(pos);
					if (path.size() > 0 && item.equals(PARENT_FOLDER)) {
						SelectFileActivity.path.pop();
						changePath();
						return;
					}
					File itemFile = new File(item);
					if (itemFile.isDirectory()) {
						path.add(item);
						changePath();
					} else {
						setResult(Activity.RESULT_OK,
								(new Intent()).setAction(item));
						Tools.setStringStackPref(SelectFileActivity.this,
								lastPathKey, path);
						finish();
					}
				}
			}
		});
	}

	private String getFullPath() {
		String pathName;
		if (path.size() > 0) {
			pathName = path.lastElement();
		} else {
			pathName = Environment.getExternalStorageDirectory()
					+ File.separator;
		}
		return pathName;
	}

	private List<String> changePath() {
		setTitlePath();
		listAdapter.clear();
		List<String> newFileNames = findFilesInDirectory(getFullPath(),
				this.extension);
		if (path.size() > 0) {
			listAdapter.add(PARENT_FOLDER);
		}
		for (String fileName : newFileNames) {
			listAdapter.add(fileName);
		}
		listAdapter.notifyDataSetChanged();
		// scroll to top
		getListView().setSelectionAfterHeaderView();
		return newFileNames;
	}

	private void setTitlePath() {
		String breadcrumb = "Path: SD Card/";
		for (String crumb : path) {
			breadcrumb += new File(crumb).getName() + "/";
		}
		setTitle(breadcrumb);
	}

	private List<String> findFilesInDirectory(String dirName,
			final String extension) {
		File dir = new File(dirName);
		File[] files = dir.listFiles(new FileFilter() {
			public boolean accept(File pathname) {
				for (String ex : extension.split("\\|")) {
					if (pathname.isFile()
							&& (pathname.getName().toLowerCase()
									.endsWith(ex.toLowerCase()) || extension
									.equals("*"))) {
						return true;
					}
				}
				return false;
			}
		});
		if (files == null) {
			files = new File[0];
		}
		File[] dirs = dir.listFiles(new FileFilter() {
			public boolean accept(File pathname) {
				return pathname.isDirectory();
			}
		});
		if (dirs == null) {
			dirs = new File[0];
		}
		String[] fileNames = new String[files.length];
		for (int i = 0; i < files.length; i++) {
			fileNames[i] = files[i].getAbsolutePath();
		}
		String[] dirNames = new String[dirs.length];
		for (int i = 0; i < dirs.length; i++) {
			dirNames[i] = dirs[i].getAbsolutePath();
		}
		Arrays.sort(dirNames, String.CASE_INSENSITIVE_ORDER);
		Arrays.sort(fileNames, String.CASE_INSENSITIVE_ORDER);
		List<String> resultList = new ArrayList<String>(Arrays.asList(dirNames));
		resultList.addAll(Arrays.asList(fileNames));
		return resultList;
	}

}
