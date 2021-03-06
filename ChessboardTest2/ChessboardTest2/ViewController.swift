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
    let recordSwitch: UISwitch = UISwitch()
    
    private let context = CIContext()
    private var startTime: CFTimeInterval!
    private let saveCountLabel: UILabel = UILabel()
    private let modeStatus: UILabel = UILabel()
    
    let totalModeCnt: Int = 5      // default, chessboard, marker, SURFNet, SURF
    var mode: Int = 0
    
    //MobileNetV2
    let net = AR3DVision()
    let net_SURF = AR3DVision_SURF()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let uuid_key = UUID().uuidString + "_dataset"
        OpenCVWrapper.initDescManager(filePath(forKey: "mtx.xml")!.absoluteURL.path, dataPath: filePath(forKey: uuid_key)!.absoluteURL.path, descPath: filePath(forKey: "base_desc.xml")!.absoluteURL.path)
        
        let gap : CGFloat = (UIScreen.main.bounds.size.height - 640) / 2
        
        startTime = CACurrentMediaTime()
        
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
        
        recordSwitch.layer.position = CGPoint(x: (UIScreen.main.bounds.width - saveButton.layer.position.x)/2 - 30, y: gap + 640)
        
        saveCountLabel.frame = CGRect(x: 0, y: 0, width: 100, height: 40)
        saveCountLabel.backgroundColor = UIColor.clear
        saveCountLabel.textAlignment = .center
        saveCountLabel.text = "0"
        saveCountLabel.textColor = UIColor.green
        saveCountLabel.layer.position = CGPoint(x: self.view.frame.width/7*6, y: gap)
        saveCountLabel.isHidden = true
        
        modeStatus.frame = CGRect(x: 0, y: 0, width: 100, height: 40)
        modeStatus.backgroundColor = UIColor.gray
        modeStatus.layer.masksToBounds = true
        modeStatus.textAlignment = .center
        modeStatus.text = "Default"
        modeStatus.textColor = UIColor.white
        modeStatus.layer.cornerRadius = 20.0
        modeStatus.layer.position = CGPoint(x: self.view.frame.width/2, y: gap)
        
        //Add a view on top of the cameras' view
        boxView = UIView(frame: self.view.frame)
        
        view.addSubview(boxView)
        view.addSubview(saveButton)
        view.addSubview(modeButton)
        view.addSubview(recordSwitch)
        view.addSubview(saveCountLabel)
        view.addSubview(modeStatus)
        
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
        
        save(image: OpenCVWrapper.makeMarkerImage(originImg), forKey: uuid + "origin")
        save(image: OpenCVWrapper.makeBlobLabel(originImg), forKey: uuid + "blob")
        save(image: OpenCVWrapper.makeCoverMarkerImage(originImg), forKey: uuid + "Cover")
        save(image: OpenCVWrapper.makeCornerImage(originImg), forKey: uuid + "Corner")
        save(image: OpenCVWrapper.maskSURFmaskImage(originImg), forKey: uuid + "mask")
        
        messageBox(messageTitle: "Success", messageAlert: "Save new image successfully", messageBoxStyle: .alert, alertActionStyle: .cancel) {
            self.dismiss(animated: true, completion: nil)
        }
    }
    
    @objc func onClickModeButton(sender: UIButton){
        OpenCVWrapper.clearData()
        
        mode = (mode + 1) % totalModeCnt
        
        if(mode == 2 || mode == 4){
            saveCountLabel.isHidden = false
        }else{
            saveCountLabel.isHidden = true
        }
        
        switch mode{
        case 0:
            modeStatus.text = "Default"
        case 1:
            modeStatus.text = "Chessboard"
        case 2:
            modeStatus.text = "Marker"
        case 3:
            modeStatus.text = "SURF Net"
        case 4:
            modeStatus.text = "SURF"
        default:
            modeStatus.text = "Default"
            mode = 0
        }
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

                if(self.recordSwitch.isOn){
                    let interval = Double(CACurrentMediaTime() - self.startTime)

                    if(interval >= 0.08){
                        let uuid = UUID().uuidString
                        let success = OpenCVWrapper.saveData(uiImage, forKey: uuid + ".png")

                        if(success){
                            save(image: OpenCVWrapper.makeCoverMarkerImage(uiImage), forKey: uuid)

                            self.saveCountLabel.text = String(format:"%d", OpenCVWrapper.getDataCount())

                            self.startTime = CACurrentMediaTime()
                        }
                    }
                }
//                dstImg = OpenCVWrapper.makeBlobLabel(uiImage)
            case 3:
                let pts = OpenCVWrapper.getSURFPos(uiImage)
                
                if let prediction = try? net_SURF.prediction(Placeholder: pts) {
                    let result = prediction.fully_connected_4_BiasAdd
                    
                    OpenCVWrapper.drawAxis(dstImg, result: result)
                }

            case 4:
                dstImg = OpenCVWrapper.makeCornerImage(uiImage)
                
                if(self.recordSwitch.isOn){
                    let interval = Double(CACurrentMediaTime() - self.startTime)

                    if(interval >= 0.08){
                        let uuid = UUID().uuidString
                        let success = OpenCVWrapper.saveSURFData(uiImage, forKey: uuid)

                        if(success){
                            self.saveCountLabel.text = String(format:"%d", OpenCVWrapper.getSURFDataCount())

                            self.startTime = CACurrentMediaTime()
                        }
                    }
                }
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

extension UIImage {
    func pixelBuffer() -> CVPixelBuffer? {
        let width = self.size.width
        let height = self.size.height
        let attrs = [kCVPixelBufferCGImageCompatibilityKey: kCFBooleanTrue,
                     kCVPixelBufferCGBitmapContextCompatibilityKey: kCFBooleanTrue] as CFDictionary
        var pixelBuffer: CVPixelBuffer?
        let status = CVPixelBufferCreate(kCFAllocatorDefault,
                                         Int(width),
                                         Int(height),
                                         kCVPixelFormatType_32ARGB,
                                         attrs,
                                         &pixelBuffer)

        guard let resultPixelBuffer = pixelBuffer, status == kCVReturnSuccess else {
            return nil
        }

        CVPixelBufferLockBaseAddress(resultPixelBuffer, CVPixelBufferLockFlags(rawValue: 0))
        let pixelData = CVPixelBufferGetBaseAddress(resultPixelBuffer)

        let rgbColorSpace = CGColorSpaceCreateDeviceRGB()
        guard let context = CGContext(data: pixelData,
                                      width: Int(width),
                                      height: Int(height),
                                      bitsPerComponent: 8,
                                      bytesPerRow: CVPixelBufferGetBytesPerRow(resultPixelBuffer),
                                      space: rgbColorSpace,
                                      bitmapInfo: CGImageAlphaInfo.noneSkipFirst.rawValue) else {
                                        return nil
        }

        context.translateBy(x: 0, y: height)
        context.scaleBy(x: 1.0, y: -1.0)

        UIGraphicsPushContext(context)
        self.draw(in: CGRect(x: 0, y: 0, width: width, height: height))
        UIGraphicsPopContext()
        CVPixelBufferUnlockBaseAddress(resultPixelBuffer, CVPixelBufferLockFlags(rawValue: 0))

        return resultPixelBuffer
    }
}
