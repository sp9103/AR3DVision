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
    var boxView:UIView!
    
    //Camera Capture requiered properties
    var videoDataOutput: AVCaptureVideoDataOutput!
    var videoDataOutputQueue: DispatchQueue!
    var previewLayer:AVCaptureVideoPreviewLayer!
    var captureDevice : AVCaptureDevice!
    let session = AVCaptureSession()
    
    private let context = CIContext()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        previewView = UIView(frame: CGRect(x: 0,
                                           y: 0,
                                           width: UIScreen.main.bounds.size.width,
                                           height: UIScreen.main.bounds.size.height))
        
        previewView.contentMode = UIView.ContentMode.scaleAspectFit
        view.addSubview(previewView)
        
        imageView.frame = CGRect(x: 0, y: 0, width: 480, height: 640)
        imageView.layer.position = CGPoint(x: self.view.frame.width/2, y: self.view.frame.height/2)
        previewView.addSubview(imageView)
        
        //Add a view on top of the cameras' view
        boxView = UIView(frame: self.view.frame)
        
        view.addSubview(boxView)
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
            
            self.imageView.image = dstImg
        }
    }

    // clean up AVCapture
    func stopCamera(){
        session.stopRunning()
    }
}

