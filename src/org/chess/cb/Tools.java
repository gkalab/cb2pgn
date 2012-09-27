package org.chess.cb;

import java.util.Stack;

import org.json.JSONArray;
import org.json.JSONException;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

public class Tools {
	public static Stack<String> getStringStackPref(Context context, String key,
			Stack<String> defaultValues) {
		SharedPreferences prefs = PreferenceManager
				.getDefaultSharedPreferences(context);
		String json = prefs.getString(key, null);
		Stack<String> result = new Stack<String>();
		if (json != null) {
			try {
				JSONArray a = new JSONArray(json);
				for (int i = 0; i < a.length(); i++) {
					String url = a.optString(i);
					result.add(url);
				}
			} catch (JSONException e) {
				Log.e("PGN", "error in getStringStackPref", e);
			}
		} else if (defaultValues != null) {
			result = defaultValues;
		}
		return result;
	}

	public static void setStringStackPref(Context context, String key,
			Stack<String> values) {
		SharedPreferences prefs = PreferenceManager
				.getDefaultSharedPreferences(context);
		SharedPreferences.Editor editor = prefs.edit();
		JSONArray a = new JSONArray();
		for (int i = 0; i < values.size(); i++) {
			a.put(values.get(i));
		}
		if (!values.isEmpty()) {
			editor.putString(key, a.toString());
		} else {
			editor.putString(key, null);
		}
		editor.commit();
	}

}
