package org.chess.cb;

import java.io.File;
import java.util.List;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

public class FileListArrayAdapter extends ArrayAdapter<String> {

	public FileListArrayAdapter(Context context, int layoutResourceId,
			int textViewResourceId, List<String> fileNames) {
		super(context, layoutResourceId, textViewResourceId, fileNames);
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		View view = super.getView(position, convertView, parent);
		String item = this.getItem(position);
		if (item != null) {
			TextView label = (TextView) view
					.findViewById(R.id.select_file_label);
			File itemFile = new File(item);
			if (itemFile.isDirectory()
					|| item.equals(SelectFileActivity.PARENT_FOLDER)) {
				label.setCompoundDrawablesWithIntrinsicBounds(
						R.drawable.folder, 0, 0, 0);
			} else {
				label.setCompoundDrawablesWithIntrinsicBounds(R.drawable.file,
						0, 0, 0);
			}
			label.setText(new File(item).getName());
		}
		return view;
	}
}