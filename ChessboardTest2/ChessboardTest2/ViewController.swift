//
//  ViewController.swift
//  ChessboardTest2
//
//  Created by sungphill on 2020/04/09.
//  Copyright © 2020 sungphill. All rights reserved.
//

import UIKit
import AVFoundation

class ViewController: UIViewController {
    var previewView : UIView!
    let imageView: UIImageView = UIImageView()
    var originImg: UIImage!
    var boxView:UIView!
    
    //Camera Capture requiered properties
    var videoDataOutput: AVCaptureVideoDataOutput!
    var videoDataOutputQueue: DispatchQueue!
    var previewLayer:AVCaptureVideoPreviewLayer!
    var captureDevice : AVCaptureDevice!
    let session = AVCaptureSession()
    
    let saveButton: UIButton = UIButton()
    let modeButton: UIButton = UIButton()
    
    private let context = CIContext()
    
    let totalModeCnt: Int = 3
    var mode: Int = 0
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let gap : CGFloat = (UIScreen.main.bounds.size.height - 640) / 2
        
        previewView = UIView(frame: CGRect(x: 0,
                                           y: 0,
                                           width: UIScreen.main.bounds.size.width,
                                           height: UIScreen.main.bounds.size.height))
        
        previewView.contentMode = UIView.ContentMode.scaleAspectFit
        view.addSubview(previewView)
        
        imageView.frame = CGRect(x: 0, y: 0, width: 480, height: 640)
        imageView.layer.position = CGPoint(x: self.view.frame.width/2, y: self.view.frame.height/2)
        previewView.addSubview(imageView)
        
        saveButton.frame = CGRect(x: 0, y: 0, width: 80, height: 40)
        saveButton.backgroundColor = UIColor.red
        saveButton.layer.masksToBounds = true
        saveButton.setTitle("Save", for: .normal)
        saveButton.setTitleColor(UIColor.white, for: .normal)
        saveButton.layer.cornerRadius = 20.0
        saveButton.layer.position = CGPoint(x: self.view.frame.width/2, y: gap + 640)
        saveButton.addTarget(self, action: #selector(self.onClickSaveButton(sender:)), for: .touchUpInside)
        
        modeButton.frame = CGRect(x: 0, y: 0, width: 80, height: 40)
        modeButton.backgroundColor = UIColor.orange
        modeButton.layer.masksToBounds = true
        modeButton.setTitle("Mode", for: .normal)
        modeButton.setTitleColor(UIColor.white, for: .normal)
        modeButton.layer.cornerRadius = 20.0
        modeButton.layer.position = CGPoint(x: (saveButton.layer.position.x + UIScreen.main.bounds.width)/2 + 30, y: gap + 640)
        modeButton.addTarget(self, action: #selector(self.onClickModeButton(sender:)), for: .touchUpInside)
        
        //Add a view on top of the cameras' view
        boxView = UIView(frame: self.view.frame)
        
        view.addSubview(boxView)
        view.addSubview(saveButton)
        view.addSubview(modeButton)
        
        self.setupAVCapture()
    }
    
    override var shouldAutorotate: Bool {
           if (UIDevice.current.orientation == UIDeviceOrientation.landscapeLeft ||
           UIDevice.current.orientation == UIDeviceOrientation.landscapeRight ||
           UIDevice.current.orientation == UIDeviceOrientation.unknown) {
               return false
           }
           else {
               return true
           }
       }
    
    @objc func onClickSaveButton(sender: UIButton){
        let uuid = UUID().uuidString
        
        save(image: originImg, forKey: uuid)
        
        messageBox(messageTitle: "Success", messageAlert: "Save new image successfully", messageBoxStyle: .alert, alertActionStyle: .cancel) {
            self.dismiss(animated: true, completion: nil)
        }
    }
    
    @objc func onClickModeButton(sender: UIButton){
        mode = (mode + 1) % totalModeCnt
    }
        
    func messageBox(messageTitle: String, messageAlert: String, messageBoxStyle: UIAlertController.Style, alertActionStyle: UIAlertAction.Style, completionHandler: @escaping () -> Void)
    {
        let alert = UIAlertController(title: messageTitle, message: messageAlert, preferredStyle: messageBoxStyle)

        let okAction = UIAlertAction(title: "Ok", style: alertActionStyle) { _ in
            completionHandler() // This will only get called after okay is tapped in the alert
        }

        alert.addAction(okAction)

        present(alert, animated: true, completion: nil)
    }
}

// AVCaptureVideoDataOutputSampleBufferDelegate protocol and related methods
extension ViewController:  AVCaptureVideoDataOutputSampleBufferDelegate{
     func setupAVCapture(){
        session.sessionPreset = AVCaptureSession.Preset.vga640x480
        guard let device = AVCaptureDevice
        .default(AVCaptureDevice.DeviceType.builtInWideAngleCamera,
                 for: .video,
                 position: AVCaptureDevice.Position.back) else {
                            return
        }
        captureDevice = device
        beginSession()
    }

    func beginSession(){
        var deviceInput: AVCaptureDeviceInput!

        do {
            deviceInput = try AVCaptureDeviceInput(device: captureDevice)
            guard deviceInput != nil else {
                print("error: cant get deviceInput")
                return
            }

            if self.session.canAddInput(deviceInput){
                self.session.addInput(deviceInput)
            }

            videoDataOutput = AVCaptureVideoDataOutput()
            videoDataOutput.alwaysDiscardsLateVideoFrames=true
            videoDataOutputQueue = DispatchQueue(label: "VideoDataOutputQueue")
            videoDataOutput.setSampleBufferDelegate(self, queue:self.videoDataOutputQueue)

            if session.canAddOutput(self.videoDataOutput){
                session.addOutput(self.videoDataOutput)
            }

            videoDataOutput.connection(with: .video)?.isEnabled = true

            session.startRunning()
        } catch let error as NSError {
            deviceInput = nil
            print("error: \(error.localizedDescription)")
        }
    }

    private func imageFromSampleBuffer(sampleBuffer: CMSampleBuffer) -> UIImage? {
        guard let imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer) else { return nil }
        let ciImage = CIImage(cvPixelBuffer: imageBuffer)
        guard let cgImage = context.createCGImage(ciImage, from: ciImage.extent) else { return nil }
        return UIImage(cgImage: cgImage)
    }
    
    func captureOutput(_ output: AVCaptureOutput, didOutput sampleBuffer: CMSampleBuffer, from connection: AVCaptureConnection) {
        // do stuff here
        connection.videoOrientation = .portrait
        
        guard let uiImage = imageFromSampleBuffer(sampleBuffer: sampleBuffer) else { return }
        
        DispatchQueue.main.sync {
            var dstImg : UIImage = uiImage
            self.originImg = uiImage
            
            switch self.mode{
            case 1:
                dstImg = OpenCVWrapper.makeChessboardImage(uiImage)
            case 2:
                dstImg = OpenCVWrapper.makeMarkerImage(uiImage)
            default:
                self.mode = 0
            }
            
            self.imageView.image = dstImg
        }
    }

    // clean up AVCapture
    func stopCamera(){
        session.stopRunning()
    }
}

