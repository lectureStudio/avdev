/*
 * Copyright 2016 Alex Andres
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.lecturestudio.avdev;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.color.ColorSpace;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.ComponentColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferByte;
import java.awt.image.Raster;
import java.awt.image.WritableRaster;
import java.lang.System.Logger;
import java.lang.System.Logger.Level;
import java.util.List;
import java.util.Locale;

import javax.swing.DefaultComboBoxModel;
import javax.swing.DefaultListCellRenderer;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSlider;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;

import org.lecturestudio.avdev.PictureFormat.PixelFormat;

public class CameraFrame extends JFrame implements HotplugListener, VideoSink {

	private static final long serialVersionUID = -9172715679498210501L;

	private static final Logger LOG = System.getLogger(CameraFrame.class.getName());

	private enum State { Idle, Capturing }
	
	private State state = State.Idle;
	
	/* UI Components */
	private JComboBox<VideoCaptureDevice> deviceCombo;
	private JComboBox<PictureFormat> formatCombo;
	private JButton captureButton;
	private JPanel controlPanel;
	private JLabel fpsLabel;
	private JComponent canvas;
	
	private long startCapture;
	private long framesCaptured;
	
	/* Buffered Image */
	private BufferedImage image;
	private byte[] imageBuffer;
	
	/* AVdev related */
	private PictureFormat format = new PictureFormat(PixelFormat.RGB24, 640, 480);
	private VideoCaptureDevice device;
	private VideoOutputStream stream;


	public CameraFrame(String title) {
		super(title);

//		try {
//			AVdev.addLogger(new SystemLogger(LOG));
//		}
//		catch (Exception e) {
//			LOG.log(Level.ERROR, e);
//		}
		
		VideoDeviceManager manager = VideoDeviceManager.getInstance();
		manager.attachHotplugListener(this);

		initUI();
		initModel();
	}

	@Override
	public void deviceConnected(Device device) {
		System.out.println("Connected: " + device);

		updateList();
	}

	@Override
	public void deviceDisconnected(Device device) {
		System.out.println("Disconnected: " + device);

		updateList();
	}

	private void updateList() {
		SwingUtilities.invokeLater(this::initModel);
	}
	
	@Override
	public void write(byte[] data, int length) {
		if (startCapture == 0) {
			// On first frame.
			startCapture = System.currentTimeMillis();
		}
		
		// Copy pixels.
		System.arraycopy(data, 0, imageBuffer, 0, imageBuffer.length);
		// Draw asynchronous.
		SwingUtilities.invokeLater(this::showImage);
	}
	
	private void initUI() {
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
		addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				System.exit(0);
			}
			
		});
		
		canvas = new JComponent() {
			
			private static final long serialVersionUID = 382462149799608343L;

			@Override
			public void update(Graphics g) {
				paint(g);
			}

			@Override
			public void paintComponent(Graphics g) {
				if (image == null)
					return;
				
				Graphics2D g2 = (Graphics2D) g;
				g2.drawImage(image, 0, 0, null);
				
			}
		};
		
		GridBagConstraints constraints = new GridBagConstraints();
		constraints.gridx = 0;
		constraints.gridy = 0;
		constraints.insets = new Insets(5, 5, 5, 5);
		
		JPanel topPanel = new JPanel(new GridBagLayout());
		
		controlPanel = new JPanel(new GridBagLayout());
		
		JScrollPane controlScrollPane = new JScrollPane(controlPanel);
		
		deviceCombo = new JComboBox<>();
		deviceCombo.setRenderer(new DefaultListCellRenderer() {

			private static final long serialVersionUID = 4490500213931134391L;
			
			public Component getListCellRendererComponent(JList<?> list, Object value, int index, boolean isSelected, boolean cellHasFocus) {
				super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);

				if (value != null) {
					Device device = (Device) value;
					setText(device.getName());
				}

				return this;
			}
		});
		deviceCombo.addActionListener(event -> {
			JComboBox<?> combo = (JComboBox<?>) event.getSource();
			selectDevice((VideoCaptureDevice) combo.getSelectedItem());
		});
		
		captureButton = new JButton("Start");
		captureButton.addActionListener(e -> {
			switch (state) {
				case Idle:
					startCapture();
					break;
				case Capturing:
					stopCapture();
					break;
			}
		});
		
		formatCombo = new JComboBox<>();
		formatCombo.setRenderer(new DefaultListCellRenderer() {

			private static final long serialVersionUID = -4350380712714251181L;

			public Component getListCellRendererComponent(JList<?> list, Object value, int index, boolean isSelected, boolean cellHasFocus) {
				super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);

				if (value != null) {
					PictureFormat format = (PictureFormat) value;
					setText(format.toString());
				}

				return this;
			}
		});
		formatCombo.addActionListener(event -> {
			JComboBox<?> combo = (JComboBox<?>) event.getSource();
			selectFormat((PictureFormat) combo.getSelectedItem());
		});
		
		fpsLabel = new JLabel("FPS: ");
		
		constraints.weightx = 1.0;
		constraints.fill = GridBagConstraints.HORIZONTAL;
		topPanel.add(deviceCombo, constraints);
		constraints.gridx++;
		topPanel.add(formatCombo, constraints);
		constraints.gridx++;
		constraints.weightx = 0.0;
		topPanel.add(captureButton, constraints);
		constraints.gridx = 0;
		constraints.gridy++;
		topPanel.add(fpsLabel, constraints);
		
		getContentPane().add(topPanel, BorderLayout.NORTH);
		getContentPane().add(canvas, BorderLayout.CENTER);
		getContentPane().add(controlScrollPane, BorderLayout.EAST);
	}

	private void initModel() {
		VideoDeviceManager manager = VideoDeviceManager.getInstance();
		List<VideoCaptureDevice> captureDevices = manager.getVideoCaptureDevices();
		VideoCaptureDevice defaultDevice = manager.getDefaultVideoCaptureDevice();

		DefaultComboBoxModel<VideoCaptureDevice> model = (DefaultComboBoxModel<VideoCaptureDevice>) deviceCombo.getModel();
		model.removeAllElements();
		
		boolean hasDevices = captureDevices != null && captureDevices.size() > 0;

		if (hasDevices) {
			for (VideoCaptureDevice device : captureDevices) {
				model.addElement(device);
			}
			
			if (defaultDevice == null) {
				defaultDevice = captureDevices.get(0);
			}
			model.setSelectedItem(defaultDevice);
		}

		captureButton.setEnabled(hasDevices);
		
		pack();
	}

	private void selectDevice(VideoCaptureDevice device) {
		this.device = device;
		
		// Re-Load picture formats.
		DefaultComboBoxModel<PictureFormat> model = (DefaultComboBoxModel<PictureFormat>) formatCombo.getModel();
		model.removeAllElements();
		
		if (device != null) {
			PictureFormat defaultFormat;
			List<PictureFormat> formats = device.getPictureFormats();
			
			if (formats != null && formats.size() > 0) {
				for (PictureFormat format : formats) {
					model.addElement(format);
				}

				defaultFormat = formats.get(0);

				model.setSelectedItem(defaultFormat);
			}
		}
		
		// Re-Load controls.
		controlPanel.removeAll();
		
		if (device != null) {
			GridBagConstraints constraints = new GridBagConstraints();
			constraints.anchor = GridBagConstraints.NORTH;
			constraints.fill = GridBagConstraints.HORIZONTAL;
			constraints.gridx = 0;
			constraints.gridy = 0;
			constraints.insets = new Insets(3, 5, 3, 5);
			
			JButton resetControlsButton = new JButton("Reset Controls");
			resetControlsButton.addActionListener(e -> resetControls());
			
			controlPanel.add(resetControlsButton, constraints);
			constraints.gridy++;

			List<PictureControl> picControls = device.getPictureControls();
			for (PictureControl control : picControls) {
				switch (control.getType()) {
					case PowerLineFrequency:
						createPowerLineFrequencyControl(control, constraints);
						break;
						
					case BacklightCompensation:
					case ColorEnable:
						createCheckControl(control, constraints);
						break;
						
					default:
						createSliderControl(control, constraints);
				}
			}

			List<CameraControl> camControls = device.getCameraControls();
			for (CameraControl control : camControls) {
				switch (control.getType()) {
					case AutoExposurePriority:
					case ScanMode:
					case Privacy:
						createCheckControl(control, constraints);
						break;
					
					case LedMode:
						createLedModeControl(control, constraints);
						break;
						
					default:
						createSliderControl(control, constraints);
				}
			}
		}
		
		pack();
	}

	private void selectFormat(PictureFormat format) {
		if (format == null) {
			return;
		}
		
		// Keep RGB format, since pixel format conversion is done by AVdev.
		this.format = format;
	}

	private void resetControls() {
		List<PictureControl> picControls = device.getPictureControls();
		for (PictureControl control : picControls) {
			try {
				device.setPictureControlValue(control.getType(), control.getDefaultValue());
			}
			catch (Exception e) {
				e.printStackTrace();
			}
		}

		List<CameraControl> camControls = device.getCameraControls();
		for (CameraControl control : camControls) {
			try {
				device.setCameraControlValue(control.getType(), control.getDefaultValue());
			}
			catch (Exception e) {
				e.printStackTrace();
			}
		}
		
		selectDevice(device);
	}

	private void startCapture() {
		try {
			createBufferedImage(format.getWidth(), format.getHeight());
			setCanvasSize(format.getWidth(), format.getHeight());
			
			device.setPictureFormat(format);
			device.setFrameRate(30);
			
			stream = device.createOutputStream(this);
			stream.open();
			stream.start();
			
			state = State.Capturing;
			
			captureButton.setText("Stop");
		}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
	}

	private void stopCapture() {
		if (stream == null) {
			return;
		}

		try {
			stream.stop();
			stream.close();

			startCapture = 0;
			framesCaptured = 0;

			state = State.Idle;

			captureButton.setText("Start");
		}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
	}

	private void setCanvasSize(int width, int height) {
		Dimension d = canvas.getSize();

		if (d.width == width && d.height == height) {
			return;
		}

		canvas.setPreferredSize(new Dimension(width, height));
		canvas.setSize(new Dimension(width, height));
		pack();
	}

	private void showImage() {
		framesCaptured++;
		
		float fps = framesCaptured * 1000.F / (System.currentTimeMillis() - startCapture);
		fpsLabel.setText(String.format(Locale.US, "%s: %.2f", "FPS", fps));
		
		canvas.repaint();
	}

	private void createBufferedImage(int width, int height) {
		if (image != null) {
			image = null;
			imageBuffer = null;
		}
		
		int bytesPerPixel = 3;
		int bufferSize = width * height * bytesPerPixel;
		
		DataBufferByte dataBuffer = new DataBufferByte(bufferSize);
		
		WritableRaster raster = Raster.createInterleavedRaster(dataBuffer,
				width,
                height,
                width * bytesPerPixel,
                bytesPerPixel,
                new int[] { 0, 1, 2 },
                null);

		ColorModel colorModel = new ComponentColorModel(ColorSpace.getInstance(ColorSpace.CS_sRGB),
				new int[] { 8, 8, 8 },
				false,
				false,
				ComponentColorModel.OPAQUE, DataBuffer.TYPE_BYTE);
		
		image = new BufferedImage(colorModel, raster, false, null);
		imageBuffer = ((DataBufferByte) image.getRaster().getDataBuffer()).getData();
	}

	private void createPowerLineFrequencyControl(final PictureControl control, GridBagConstraints constraints) {
		controlPanel.add(new JLabel(control.getName()), constraints);
		
		JComboBox<Integer> combo = new JComboBox<>(new Integer[] { 0, 1, 2 });
		combo.setRenderer(new DefaultListCellRenderer() {

			private static final long serialVersionUID = -7968744737904640624L;

			public Component getListCellRendererComponent(JList<?> list, Object value, int index, boolean isSelected, boolean cellHasFocus) {
				super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);

				if (value != null) {
					String str = null;
					int id = (int) value;
					
					switch (id) {
						case 0:
							str = "Disabled";
							break;
						case 1:
							str = "50 Hz";
							break;
						case 2:
							str = "60 Hz";
							break;
					}
					
					setText(str);
				}

				return this;
			}
		});
		
		try {
			combo.setSelectedItem((int) device.getPictureControlValue(control.getType()));
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
		combo.addActionListener(event -> {
			JComboBox<?> combo1 = (JComboBox<?>) event.getSource();
			int value = (int) combo1.getSelectedItem();

			try {
				device.setPictureControlValue(control.getType(), value);
			}
			catch (Exception e) {
				e.printStackTrace();
			}
		});
		
		constraints.gridy++;
		controlPanel.add(combo, constraints);
		constraints.gridy++;
	}

	private void createAutoExposureControl(final CameraControl control, GridBagConstraints constraints) {
		controlPanel.add(new JLabel(control.getName()), constraints);
		
		JComboBox<Integer> combo = new JComboBox<>(new Integer[] { 0, 1, 2, 3 });
		combo.setRenderer(new DefaultListCellRenderer() {

			private static final long serialVersionUID = 1L;

			public Component getListCellRendererComponent(JList<?> list, Object value, int index, boolean isSelected, boolean cellHasFocus) {
				super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);

				if (value != null) {
					String str = null;
					int id = (int) value;
					
					switch (id) {
						case 0:
							str = "Auto";
							break;
						case 1:
							str = "Manual";
							break;
						case 2:
							str = "Shutter Priority";
							break;
						case 3:
							str = "Aperture Priority";
							break;
					}
					
					setText(str);
				}

				return this;
			}
		});
		
		try {
			combo.setSelectedItem((int) device.getCameraControlValue(control.getType()));
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
		combo.addActionListener(event -> {
			JComboBox<?> combo1 = (JComboBox<?>) event.getSource();
			int value = (int) combo1.getSelectedItem();

			try {
				device.setCameraControlValue(control.getType(), value);
			}
			catch (Exception e) {
				e.printStackTrace();
			}
		});
		
		constraints.gridy++;
		controlPanel.add(combo, constraints);
		constraints.gridy++;
	}

	private void createLedModeControl(final CameraControl control, GridBagConstraints constraints) {
		controlPanel.add(new JLabel(control.getName()), constraints);
		
		JComboBox<Integer> combo = new JComboBox<>(new Integer[] { 0, 1, 2, 3 });
		combo.setRenderer(new DefaultListCellRenderer() {

			private static final long serialVersionUID = 1L;

			public Component getListCellRendererComponent(JList<?> list, Object value, int index, boolean isSelected, boolean cellHasFocus) {
				super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);

				if (value != null) {
					String str = null;
					int id = (int) value;
					
					switch (id) {
						case 0:
							str = "Off";
							break;
						case 1:
							str = "On";
							break;
						case 2:
							str = "Blinking";
							break;
						case 3:
							str = "Auto";
							break;
					}
					
					setText(str);
				}

				return this;
			}
		});
		
		try {
			combo.setSelectedItem((int) device.getCameraControlValue(control.getType()));
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
		combo.addActionListener(event -> {
			JComboBox<?> combo1 = (JComboBox<?>) event.getSource();
			int value = (int) combo1.getSelectedItem();

			try {
				device.setCameraControlValue(control.getType(), value);
			}
			catch (Exception e) {
				e.printStackTrace();
			}
		});
		
		constraints.gridy++;
		controlPanel.add(combo, constraints);
		constraints.gridy++;
	}

	private void createSliderControl(final PictureControl control, GridBagConstraints constraints) {
		if (control.hasAutoMode()) {
			createCheckControl(control, constraints);
			
			if (control.getMinValue() == 0 && control.getMaxValue() == 1) {
				// No range values available.
				return;
			}
		}
		
		controlPanel.add(new JLabel(control.getName()), constraints);
		
		JSlider slider = new JSlider();
		slider.setMinimum((int) control.getMinValue());
		slider.setMaximum((int) control.getMaxValue());
		slider.setMinorTickSpacing((int) control.getStepValue());
		slider.setMajorTickSpacing((int) control.getStepValue());
		slider.setSnapToTicks(true);
		
		try {
			slider.setValue((int) device.getPictureControlValue(control.getType()));
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
		slider.addChangeListener(event -> {
			JSlider source = (JSlider) event.getSource();
			if (!source.getValueIsAdjusting()) {
				int value = source.getValue();
				try {
					device.setPictureControlValue(control.getType(), value);
				}
				catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
		
		constraints.gridy++;
		
		controlPanel.add(slider, constraints);
		constraints.gridy++;
	}

	private void createSliderControl(final CameraControl control, GridBagConstraints constraints) {
		if (control.hasAutoMode()) {
			createCheckControl(control, constraints);
			
			if (control.getMinValue() == 0 && control.getMaxValue() == 1) {
				// No range values available.
				return;
			}
		}
		
		controlPanel.add(new JLabel(control.getName()), constraints);
		
		JSlider slider = new JSlider();
		slider.setMinimum((int) control.getMinValue());
		slider.setMaximum((int) control.getMaxValue());
		slider.setMinorTickSpacing((int) control.getStepValue());
		slider.setMajorTickSpacing((int) control.getStepValue());
		slider.setSnapToTicks(true);
		
		try {
			slider.setValue((int) device.getCameraControlValue(control.getType()));
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
		slider.addChangeListener(event -> {
			JSlider source = (JSlider) event.getSource();
			if (!source.getValueIsAdjusting()) {
				int value = source.getValue();
				try {
					device.setCameraControlValue(control.getType(), value);
				}
				catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
		
		constraints.gridy++;
		controlPanel.add(slider, constraints);
		constraints.gridy++;
	}

	private void createCheckControl(final PictureControl control, GridBagConstraints constraints) {
		JCheckBox check = new JCheckBox(control.getName());

		try {
			if (control.hasAutoMode()) {
				check.setSelected(device.getPictureControlAutoMode(control.getType()));
			}
			else {
				check.setSelected(device.getPictureControlValue(control.getType()) != 0);
			}
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
		check.addActionListener(event -> {
			JCheckBox source = (JCheckBox) event.getSource();
			long value = source.isSelected() ? 1 : 0;
			try {
				if (control.hasAutoMode()) {
					device.setPictureControlAutoMode(control.getType(), source.isSelected());
				}
				else {
					device.setPictureControlValue(control.getType(), value);
				}
			}
			catch (Exception e) {
				e.printStackTrace();
			}
		});
		
		constraints.gridy++;
		controlPanel.add(check, constraints);
		constraints.gridy++;
	}

	private void createCheckControl(final CameraControl control, GridBagConstraints constraints) {
		JCheckBox check = new JCheckBox(control.getName());
		
		try {
			if (control.hasAutoMode()) {
				check.setSelected(device.getCameraControlAutoMode(control.getType()));
			}
			else {
				check.setSelected(device.getCameraControlValue(control.getType()) != 0);
			}
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
		check.addActionListener(event -> {
			JCheckBox source = (JCheckBox) event.getSource();
			long value = source.isSelected() ? 1 : 0;
			try {
				if (control.hasAutoMode()) {
					device.setCameraControlAutoMode(control.getType(), source.isSelected());
				}
				else {
					device.setCameraControlValue(control.getType(), value);
				}
			}
			catch (Exception e) {
				e.printStackTrace();
			}
		});
		
		constraints.gridy++;
		controlPanel.add(check, constraints);
		constraints.gridy++;
	}
	
}
